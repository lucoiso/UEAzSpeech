// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/WavFileToTextAsync.h"
#include "HAL/PlatformFileManager.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"
#include "Async/Async.h"

UWavFileToTextAsync* UWavFileToTextAsync::WavFileToText(const UObject* WorldContextObject, const FString& FilePath, const FString& FileName, const FString& LanguageId, const bool bContinuosRecognition)
{
	UWavFileToTextAsync* const NewAsyncTask = NewObject<UWavFileToTextAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->bContinuousRecognition = bContinuosRecognition;

	return NewAsyncTask;
}

void UWavFileToTextAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.READ_EXTERNAL_STORAGE");
#endif

	Super::Activate();
}

bool UWavFileToTextAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (AzSpeech::Internal::HasEmptyParam(FilePath, FileName, LanguageId))
	{
		return false;
	}

	const FString QualifiedPath = UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName);
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*QualifiedPath))
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: File not found"), *FString(__func__));
		return false;
	}

	const std::string InFilePath = TCHAR_TO_UTF8(*QualifiedPath);
	const auto AudioConfig = Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromWavFileInput(InFilePath);
	if (!InitializeRecognizer(AudioConfig))
	{
		return false;
	}

	StartRecognitionWork();
	return true;
}