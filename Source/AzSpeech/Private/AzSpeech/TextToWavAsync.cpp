// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToWavAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"

UTextToWavAsync* UTextToWavAsync::TextToWav(const UObject* WorldContextObject, const FString& TextToConvert, const FString& FilePath, const FString& FileName, const FString& VoiceName, const FString& LanguageId)
{
	UTextToWavAsync* const TextToWavAsync = NewObject<UTextToWavAsync>();
	TextToWavAsync->WorldContextObject = WorldContextObject;
	TextToWavAsync->TextToConvert = TextToConvert;
	TextToWavAsync->FilePath = FilePath;
	TextToWavAsync->FileName = FileName;
	TextToWavAsync->VoiceName = AzSpeech::Internal::GetVoiceName(VoiceName);
	TextToWavAsync->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);

	return TextToWavAsync;
}

void UTextToWavAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.WRITE_EXTERNAL_STORAGE");
#endif

	Super::Activate();
}

void UTextToWavAsync::StartAzureTaskWork_Internal()
{
	if (TextToConvert.IsEmpty() || VoiceName.IsEmpty() || FilePath.IsEmpty() || FileName.IsEmpty() || LanguageID.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
		return;
	}

	if (!UAzSpeechHelper::CreateNewDirectory(FilePath))
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Failed to create directory"), *FString(__func__));
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, this]
	{
		const TFuture<bool> TextToWavAsyncWork = Async(EAsyncExecution::Thread, [=]() -> bool
		{
			const std::string InConvertStr = TCHAR_TO_UTF8(*TextToConvert);
			const std::string InLanguageIDStr = TCHAR_TO_UTF8(*LanguageID);
			const std::string InNameIDStr = TCHAR_TO_UTF8(*VoiceName);
			const std::string InFilePathStr = TCHAR_TO_UTF8(*UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName));

			return DoAzureTaskWork_Internal(InConvertStr, InLanguageIDStr, InNameIDStr, InFilePathStr);
		});

		if (!TextToWavAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
			return;
		}

		const bool bOutputValue = TextToWavAsyncWork.Get();
		AsyncTask(ENamedThreads::GameThread, [=]() { TaskCompleted.Broadcast(bOutputValue); });

		if (bOutputValue)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(FuncName));
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed"), *FString(FuncName));
		}
	});
}

bool UTextToWavAsync::DoAzureTaskWork_Internal(const std::string& InStr, const std::string& InLanguageID, const std::string& InVoiceName, const std::string& InFilePath)
{
	const auto AudioConfig = AudioConfig::FromWavFileOutput(InFilePath);
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig, InLanguageID, InVoiceName);

	if (!SynthesizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Failed to proceed with task: SynthesizerObject is null"), *FString(__func__));
		return false;
	}

	const auto SynthesisResult = SynthesizerObject->SpeakTextAsync(InStr).get();

	return AzSpeech::Internal::ProcessSynthesizResult(SynthesisResult);
}
