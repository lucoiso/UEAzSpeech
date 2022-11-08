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
	UWavToTextAsync* const NewAsyncTask = NewObject<UWavToTextAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);
	NewAsyncTask->bContinuousRecognition = bContinuosRecognition;

	return NewAsyncTask;
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
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Missing parameters"), *FString(__func__));
		return false;
	}

	const FString QualifiedPath = UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName);
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*QualifiedPath))
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: File not found"), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: Initializing task"), *FString(__func__));

	const std::string InFilePath = TCHAR_TO_UTF8(*QualifiedPath);
	const std::string InLanguage = TCHAR_TO_UTF8(*LanguageID);

	const auto AudioConfig = AudioConfig::FromWavFileInput(InFilePath);
	RecognizerObject = AzSpeech::Internal::GetAzureRecognizer(AudioConfig, InLanguage);

	if (!RecognizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Failed to proceed with task: RecognizerObject is null"), *FString(__func__));
		return false;
	}

	ApplyExtraSettings();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]
	{
		if (bContinuousRecognition)
		{
			RecognizerObject->StartContinuousRecognitionAsync().wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
		}
		else
		{
			RecognizerObject->RecognizeOnceAsync().wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
		}
	});

	return true;
}