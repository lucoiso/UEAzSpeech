// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SpeechToTextAsync.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include "Async/Async.h"

#if PLATFORM_ANDROID
// Only used to check android permission
#include "AzSpeech/AzSpeechHelper.h"
#endif

USpeechToTextAsync* USpeechToTextAsync::SpeechToText(const UObject* WorldContextObject, const FString& LanguageId, const bool bContinuosRecognition)
{
	USpeechToTextAsync* const NewAsyncTask = NewObject<USpeechToTextAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->bContinuousRecognition = bContinuosRecognition;

	return NewAsyncTask;
}

void USpeechToTextAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.RECORD_AUDIO");
#endif

	Super::Activate();
}

bool USpeechToTextAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (AzSpeech::Internal::HasEmptyParam(LanguageId))
	{
		return false;
	}

	const auto AudioConfig = Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromDefaultMicrophoneInput();
	if (!InitializeRecognizer(AudioConfig))
	{
		return false;
	}

	StartRecognitionWork();
	return true;
}
