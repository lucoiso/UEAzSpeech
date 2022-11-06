// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/WavToTextAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "HAL/PlatformFileManager.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"

UWavToTextAsync* UWavToTextAsync::WavToText(const UObject* WorldContextObject, const FString& FilePath, const FString& FileName, const FString& LanguageId, const bool bContinuosRecognition)
{
	UWavToTextAsync* const WavToTextAsync = NewObject<UWavToTextAsync>();
	WavToTextAsync->WorldContextObject = WorldContextObject;
	WavToTextAsync->FilePath = FilePath;
	WavToTextAsync->FileName = FileName;
	WavToTextAsync->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);
	WavToTextAsync->bContinuousRecognition = bContinuosRecognition;

	return WavToTextAsync;
}

void UWavToTextAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.READ_EXTERNAL_STORAGE");
#endif

	Super::Activate();
}

bool UWavToTextAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (FilePath.IsEmpty() || FileName.IsEmpty() || LanguageID.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
		return false;
	}

	const FString QualifiedPath = UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName);
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*QualifiedPath))
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: File not found"), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, QualifiedPath, this]
	{
		const TFuture<std::string> WavToTextAsyncWork = Async(EAsyncExecution::Thread, [=]() -> std::string
		{
			const std::string InFilePathStr = TCHAR_TO_UTF8(*QualifiedPath);
			const std::string InLanguageIDStr = TCHAR_TO_UTF8(*LanguageID);

			return DoAzureTaskWork_Internal(InFilePathStr, InLanguageIDStr);
		});

		if (!WavToTextAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
			return;
		}

		const FString OutputValue = UTF8_TO_TCHAR(WavToTextAsyncWork.Get().c_str());

		if (!OutputValue.Equals("CONTINUOUS_RECOGNITION"))
		{
			AsyncTask(ENamedThreads::GameThread, [=]() { if (CanBroadcast()) { TaskCompleted.Broadcast(OutputValue); } });
		}

		if (!OutputValue.IsEmpty())
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: %s"), *FString(FuncName), *OutputValue);
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed"), *FString(FuncName));
		}
	});

	return true;
}

std::string UWavToTextAsync::DoAzureTaskWork_Internal(const std::string& InFilePath, const std::string& InLanguageID)
{
	const auto AudioConfig = AudioConfig::FromWavFileInput(InFilePath);
	RecognizerObject = AzSpeech::Internal::GetAzureRecognizer(AudioConfig, InLanguageID);

	if (RecognizerObject == nullptr)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Failed to proceed with task: RecognizerObject is null"), *FString(__func__));
		return std::string();
	}

	if (const auto RecognitionResult = RecognizerObject->RecognizeOnceAsync().get();
		AzSpeech::Internal::ProcessRecognitionResult(RecognitionResult))
	{
		return RecognitionResult->Text;
	}

	return std::string();
}
