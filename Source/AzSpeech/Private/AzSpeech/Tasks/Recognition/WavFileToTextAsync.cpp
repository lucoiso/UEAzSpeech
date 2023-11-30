// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Recognition/WavFileToTextAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"
#include <HAL/FileManager.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(WavFileToTextAsync)
#endif

namespace MicrosoftSpeech = Microsoft::CognitiveServices::Speech;

UWavFileToTextAsync* UWavFileToTextAsync::WavFileToText_DefaultOptions(UObject* const WorldContextObject, const FString& FilePath, const FString& FileName, const FString& Locale, const FName& PhraseListGroup)
{
    return WavFileToText_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechRecognitionOptions(*Locale), FilePath, FileName, PhraseListGroup);
}

UWavFileToTextAsync* UWavFileToTextAsync::WavFileToText_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechRecognitionOptions& RecognitionOptions, const FString& FilePath, const FString& FileName, const FName& PhraseListGroup)
{
    UWavFileToTextAsync* const NewAsyncTask = NewObject<UWavFileToTextAsync>();
    NewAsyncTask->SubscriptionOptions = SubscriptionOptions;
    NewAsyncTask->RecognitionOptions = RecognitionOptions;
    NewAsyncTask->FilePath = FilePath;
    NewAsyncTask->FileName = FileName;
    NewAsyncTask->PhraseListGroup = PhraseListGroup;
    NewAsyncTask->bIsSSMLBased = false;
    NewAsyncTask->TaskName = *FString(__func__);

    NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

    return NewAsyncTask;
}

void UWavFileToTextAsync::Activate()
{
#if PLATFORM_ANDROID
    if (!UAzSpeechHelper::CheckAndroidPermission("android.permission.READ_EXTERNAL_STORAGE"))
    {
        SetReadyToDestroy();
        return;
    }
#endif

    Super::Activate();
}

bool UWavFileToTextAsync::StartAzureTaskWork()
{
    if (!Super::StartAzureTaskWork())
    {
        return false;
    }

    if (AzSpeech::Internal::HasEmptyParam(FilePath, FileName, GetRecognitionOptions().Locale))
    {
        return false;
    }

    const FString QualifiedPath = UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName);

    if (!IFileManager::Get().FileExists(*QualifiedPath))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: File '%s' not found"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *QualifiedPath);
        return false;
    }

    if (IFileManager::Get().FileSize(*QualifiedPath) <= 0)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: File '%s' is invalid"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *QualifiedPath);
        return false;
    }

    // Try to open the file before sending to Azure - Avoid crash due to the file already being used by another proccess
    if (FArchive* const Archive = IFileManager::Get().CreateFileReader(*QualifiedPath); Archive == nullptr)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Failed to load file '%s'"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *QualifiedPath);
        return false;
    }
    else
    {
        delete Archive;
    }

    AudioConfig = MicrosoftSpeech::Audio::AudioConfig::FromWavFileInput(TCHAR_TO_UTF8(*QualifiedPath));
    StartRecognitionWork();

    return true;
}