// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/WavFileToTextAsync.h"
#include "HAL/PlatformFileManager.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"

UWavFileToTextAsync* UWavFileToTextAsync::WavFileToText(const UObject* WorldContextObject, const FString& FilePath, const FString& FileName, const FString& LanguageId)
{
	UWavFileToTextAsync* const NewAsyncTask = NewObject<UWavFileToTextAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->TaskName = *FString(__func__);

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

	if (AzSpeech::Internal::HasEmptyParam(FilePath, FileName, LanguageId))
	{
		return false;
	}

	const FString QualifiedPath = UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName);
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*QualifiedPath))
	{		
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: File '%s' not found"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *QualifiedPath);
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