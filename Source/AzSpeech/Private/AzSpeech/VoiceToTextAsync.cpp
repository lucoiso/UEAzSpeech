// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/VoiceToTextAsync.h"
#include "AzSpeechInternalFuncs.h"
#include "Async/Async.h"

#if PLATFORM_ANDROID
// Only used to check android permission
#include "AzSpeech/AzSpeechHelper.h"
#endif

UVoiceToTextAsync* UVoiceToTextAsync::VoiceToText(const UObject* WorldContextObject, const FString& LanguageId, const bool bContinuosRecognition)
{
	UVoiceToTextAsync* const NewAsyncTask = NewObject<UVoiceToTextAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->bContinuousRecognition = bContinuosRecognition;

	return NewAsyncTask;
}

void UVoiceToTextAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.RECORD_AUDIO");
#endif

	Super::Activate();
}

bool UVoiceToTextAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (HasEmptyParam(LanguageId))
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
