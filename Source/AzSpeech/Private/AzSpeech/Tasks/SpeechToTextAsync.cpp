// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SpeechToTextAsync.h"
#include "AzSpeech/AzSpeechHelper.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SpeechToTextAsync)
#endif

USpeechToTextAsync* USpeechToTextAsync::SpeechToText_DefaultOptions(UObject* WorldContextObject, const FString& LanguageID, const FString& AudioInputDeviceID, const FName PhraseListGroup)
{
	return SpeechToText_CustomOptions(WorldContextObject, FAzSpeechSettingsOptions(*LanguageID), AudioInputDeviceID, PhraseListGroup);
}

USpeechToTextAsync* USpeechToTextAsync::SpeechToText_CustomOptions(UObject* WorldContextObject, const FAzSpeechSettingsOptions& Options, const FString& AudioInputDeviceID, const FName PhraseListGroup)
{
	USpeechToTextAsync* const NewAsyncTask = NewObject<USpeechToTextAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->TaskOptions = GetValidatedOptions(Options);
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

	if (AzSpeech::Internal::HasEmptyParam(GetTaskOptions().LanguageID))
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
	
	const auto AudioConfig = IsUsingDefaultAudioInputDevice() ? Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromDefaultMicrophoneInput() : Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromMicrophoneInput(TCHAR_TO_UTF8(*DeviceInfo.GetAudioInputDeviceEndpointID()));
	StartRecognitionWork(AudioConfig);

	return true;
}
