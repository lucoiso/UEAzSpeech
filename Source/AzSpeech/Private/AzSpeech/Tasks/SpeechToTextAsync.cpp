// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SpeechToTextAsync.h"
#include "AzSpeech/AzSpeechHelper.h"

USpeechToTextAsync* USpeechToTextAsync::SpeechToText(const UObject* WorldContextObject, const FString& LanguageID, const FString& AudioInputDeviceID, const FName PhraseListGroup)
{
	USpeechToTextAsync* const NewAsyncTask = NewObject<USpeechToTextAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->AudioInputDeviceID = AudioInputDeviceID;
	NewAsyncTask->PhraseListGroup = PhraseListGroup;
	NewAsyncTask->TaskName = *FString(__func__);

	return NewAsyncTask;
}

void USpeechToTextAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.RECORD_AUDIO");
#endif

	Super::Activate();
}

bool USpeechToTextAsync::IsUsingDefaultAudioInputDevice() const
{
	return HasEmptyParameters(AudioInputDeviceID) || AudioInputDeviceID.Equals("Default", ESearchCase::IgnoreCase);
}

bool USpeechToTextAsync::StartAzureTaskWork()
{
	if (!Super::StartAzureTaskWork())
	{
		return false;
	}

	if (HasEmptyParameters(LanguageID))
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
