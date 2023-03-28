// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/TextToSoundWaveAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include <Sound/SoundWave.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(TextToSoundWaveAsync)
#endif

UTextToSoundWaveAsync* UTextToSoundWaveAsync::TextToSoundWave_DefaultOptions(UObject* WorldContextObject, const FString& SynthesisText, const FString& VoiceName, const FString& LanguageID)
{
	return TextToSoundWave_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechSynthesisOptions(*LanguageID, *VoiceName), SynthesisText);
}

UTextToSoundWaveAsync* UTextToSoundWaveAsync::TextToSoundWave_CustomOptions(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText)
{
	UTextToSoundWaveAsync* const NewAsyncTask = NewObject<UTextToSoundWaveAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SynthesisText;
	NewAsyncTask->SubscriptionOptions = SubscriptionOptions;
	NewAsyncTask->SynthesisOptions = SynthesisOptions;
	NewAsyncTask->bIsSSMLBased = false;
	NewAsyncTask->TaskName = *FString(__func__);
	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}

void UTextToSoundWaveAsync::BroadcastFinalResult()
{
	FScopeLock Lock(&Mutex);

	if (!UAzSpeechTaskStatus::IsTaskActive(this))
	{
		return;
	}

	Super::BroadcastFinalResult();

	const TArray<uint8> LastBuffer = GetAudioData();
	SynthesisCompleted.Broadcast(UAzSpeechHelper::ConvertAudioDataToSoundWave(LastBuffer));
}
