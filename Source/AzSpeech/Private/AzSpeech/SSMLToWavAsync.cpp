// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToWavAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"

USSMLToWavAsync* USSMLToWavAsync::SSMLToWav(const UObject* WorldContextObject, const FString& SSMLString, const FString& FilePath, const FString& FileName)
{
	USSMLToWavAsync* const TextToWavAsync = NewObject<USSMLToWavAsync>();
	TextToWavAsync->WorldContextObject = WorldContextObject;
	TextToWavAsync->SSMLString = SSMLString;
	TextToWavAsync->FilePath = FilePath;
	TextToWavAsync->FileName = FileName;

	return TextToWavAsync;
}

void USSMLToWavAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.WRITE_EXTERNAL_STORAGE");
#endif

	Super::Activate();
}

bool USSMLToWavAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (SSMLString.IsEmpty() || FilePath.IsEmpty() || FileName.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
		return false;
	}

	if (!UAzSpeechHelper::CreateNewDirectory(FilePath))
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Failed to create directory"), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, this]
	{
		const TFuture<bool> SSMLToWavAsyncWork = Async(EAsyncExecution::Thread, [=]() -> bool
		{
			const std::string InConvertStr = TCHAR_TO_UTF8(*SSMLString);
			const std::string InFilePathStr = TCHAR_TO_UTF8(*UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName));

			return DoAzureTaskWork_Internal(InConvertStr, InFilePathStr);
		});

		if (!SSMLToWavAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
		}

		const bool bOutputValue = SSMLToWavAsyncWork.Get();
		AsyncTask(ENamedThreads::GameThread, [=]() { if (CanBroadcast()) { SynthesisCompleted.Broadcast(bOutputValue); } });

		if (bOutputValue)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(FuncName));
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed"), *FString(FuncName));
		}
	});
	
	return true;
}

bool USSMLToWavAsync::DoAzureTaskWork_Internal(const std::string& InSSML, const std::string& InFilePath)
{
	const auto AudioConfig = AudioConfig::FromWavFileOutput(InFilePath);
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig);

	if (!SynthesizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Failed to proceed with task: SynthesizerObject is null"), *FString(__func__));
		return false;
	}
	
	CheckAndAddViseme();

	const auto SynthesisResult = SynthesizerObject->SpeakSsmlAsync(InSSML).get();

	return AzSpeech::Internal::ProcessSynthesizResult(SynthesisResult);
}
