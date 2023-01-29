// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/WavFileToTextAsync.h"
#include "AzSpeech/AzSpeechHelper.h"

#if ENGINE_MAJOR_VERSION < 5
#include <HAL/PlatformFilemanager.h>
#else
#include <HAL/PlatformFileManager.h>
#endif

UWavFileToTextAsync* UWavFileToTextAsync::WavFileToText(const UObject* WorldContextObject, const FString& FilePath, const FString& FileName, const FString& LanguageID, const FName PhraseListGroup)
{
	UWavFileToTextAsync* const NewAsyncTask = NewObject<UWavFileToTextAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->LanguageID = LanguageID;
	NewAsyncTask->PhraseListGroup = PhraseListGroup;
	NewAsyncTask->bIsSSMLBased = false;
	NewAsyncTask->TaskName = *FString(__func__);
	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}

void UWavFileToTextAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.READ_EXTERNAL_STORAGE");
#endif

	Super::Activate();
}

bool UWavFileToTextAsync::StartAzureTaskWork()
{
	if (!Super::StartAzureTaskWork())
	{
		return false;
	}
	
	if (HasEmptyParameters(FilePath, FileName, LanguageID))
	{
		return false;
	}

	const FString QualifiedPath = UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName);

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*QualifiedPath))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: File '%s' not found"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *QualifiedPath);
		return false;
	}

	if (FPlatformFileManager::Get().GetPlatformFile().FileSize(*QualifiedPath) <= 0)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: File '%s' is invalid"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *QualifiedPath);
		return false;
	}

	// Try to open the file before sending to Azure - Avoid crash due to the file already being used by another proccess
	if (IFileHandle* const FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenRead(*QualifiedPath);
		FileHandle == nullptr)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Failed to load file '%s'"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *QualifiedPath);
		return false;
	}
	else
	{
		delete FileHandle;
	}

	const auto AudioConfig = Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromWavFileInput(TCHAR_TO_UTF8(*QualifiedPath));
	StartRecognitionWork(AudioConfig);

	return true;
}