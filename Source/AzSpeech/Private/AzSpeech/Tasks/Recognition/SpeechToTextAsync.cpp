// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Recognition/SpeechToTextAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SpeechToTextAsync)
#endif

namespace MicrosoftSpeech = Microsoft::CognitiveServices::Speech;

USpeechToTextAsync* USpeechToTextAsync::SpeechToText_DefaultOptions(UObject* const WorldContextObject, const FString& Locale, const FString& AudioInputDeviceID, const FName& PhraseListGroup)
{
    return SpeechToText_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechRecognitionOptions(*Locale), AudioInputDeviceID, PhraseListGroup);
}

USpeechToTextAsync* USpeechToTextAsync::SpeechToText_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechRecognitionOptions& RecognitionOptions, const FString& AudioInputDeviceID, const FName& PhraseListGroup)
{
    USpeechToTextAsync* const NewAsyncTask = NewObject<USpeechToTextAsync>();
    NewAsyncTask->SubscriptionOptions = SubscriptionOptions;
    NewAsyncTask->RecognitionOptions = RecognitionOptions;
    NewAsyncTask->AudioInputDeviceID = AudioInputDeviceID;
    NewAsyncTask->PhraseListGroup = PhraseListGroup;
    NewAsyncTask->bIsSSMLBased = false;
    NewAsyncTask->TaskName = *FString(__func__);

    NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

    return NewAsyncTask;
}

void USpeechToTextAsync::Activate()
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

bool USpeechToTextAsync::IsUsingDefaultAudioInputDevice() const
{
    return AzSpeech::Internal::HasEmptyParam(AudioInputDeviceID) || AudioInputDeviceID.Equals("Default", ESearchCase::IgnoreCase);
}

bool USpeechToTextAsync::StartAzureTaskWork()
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