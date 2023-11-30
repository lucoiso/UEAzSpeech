// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Recognition/KeywordRecognitionAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeech/Runnables/Recognition/AzSpeechKeywordRecognitionRunnable.h"
#include "AzSpeechInternalFuncs.h"
#include <HAL/FileManager.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(KeywordRecognitionAsync)
#endif

namespace MicrosoftSpeech = Microsoft::CognitiveServices::Speech;

UKeywordRecognitionAsync* UKeywordRecognitionAsync::KeywordRecognition_DefaultOptions(UObject* const WorldContextObject, const FString& Locale, const FString& AudioInputDeviceID, const FName& PhraseListGroup)
{
    return KeywordRecognition_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechRecognitionOptions(*Locale), AudioInputDeviceID, PhraseListGroup);
}

UKeywordRecognitionAsync* UKeywordRecognitionAsync::KeywordRecognition_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechRecognitionOptions& RecognitionOptions, const FString& AudioInputDeviceID, const FName& PhraseListGroup)
{
    UKeywordRecognitionAsync* const NewAsyncTask = NewObject<UKeywordRecognitionAsync>();
    NewAsyncTask->SubscriptionOptions = SubscriptionOptions;
    NewAsyncTask->RecognitionOptions = RecognitionOptions;
    NewAsyncTask->AudioInputDeviceID = AudioInputDeviceID;
    NewAsyncTask->PhraseListGroup = PhraseListGroup;
    NewAsyncTask->bIsSSMLBased = false;
    NewAsyncTask->TaskName = *FString(__func__);

    NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

    return NewAsyncTask;
}

void UKeywordRecognitionAsync::Activate()
{
#if PLATFORM_ANDROID
    if (!UAzSpeechHelper::CheckAndroidPermission("android.permission.RECORD_AUDIO"))
    {
        SetReadyToDestroy();
        return;
    }
#endif

    Super::Activate();
}

bool UKeywordRecognitionAsync::IsUsingDefaultAudioInputDevice() const
{
    return AzSpeech::Internal::HasEmptyParam(AudioInputDeviceID) || AudioInputDeviceID.Equals("Default", ESearchCase::IgnoreCase);
}

bool UKeywordRecognitionAsync::StartAzureTaskWork()
{
    if (!Super::StartAzureTaskWork())
    {
        return false;
    }

    if (AzSpeech::Internal::HasEmptyParam(GetRecognitionOptions().Locale))
    {
        return false;
    }

    const FAzSpeechAudioInputDeviceInfo DeviceInfo = UAzSpeechHelper::GetAudioInputDeviceInfoFromID(AudioInputDeviceID);
    if (!IsUsingDefaultAudioInputDevice() && !UAzSpeechHelper::IsAudioInputDeviceIDValid(DeviceInfo.GetDeviceID()))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Audio input device %s isn't available."), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *DeviceInfo.GetAudioInputDeviceEndpointID());

        return false;
    }

    UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Using audio input device: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), IsUsingDefaultAudioInputDevice() ? *FString("Default") : *DeviceInfo.GetAudioInputDeviceEndpointID());

    AudioConfig = IsUsingDefaultAudioInputDevice() ? MicrosoftSpeech::Audio::AudioConfig::FromDefaultMicrophoneInput() : MicrosoftSpeech::Audio::AudioConfig::FromMicrophoneInput(TCHAR_TO_UTF8(*DeviceInfo.GetAudioInputDeviceEndpointID()));
    StartRecognitionWork();

    return true;
}

void UKeywordRecognitionAsync::StartRecognitionWork()
{
    const FString ModelPath = GetRecognitionOptions().KeywordRecognitionModelPath;

    if (!IFileManager::Get().FileExists(*ModelPath))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: File '%s' not found"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *ModelPath);
        SetReadyToDestroy();
        return;
    }

    if (IFileManager::Get().FileSize(*ModelPath) <= 0)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: File '%s' is invalid"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *ModelPath);
        SetReadyToDestroy();
        return;
    }

    RunnableTask = MakeUnique<FAzSpeechKeywordRecognitionRunnable>(this, AudioConfig, MicrosoftSpeech::KeywordRecognitionModel::FromFile(TCHAR_TO_UTF8(*ModelPath)));
    if (!RunnableTask)
    {
        SetReadyToDestroy();
        return;
    }

    RunnableTask->StartAzSpeechRunnableTask();
}