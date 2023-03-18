// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/TextToSoundWaveAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include <Sound/SoundWave.h>
#include <Async/Async.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(TextToSoundWaveAsync)
#endif

UTextToSoundWaveAsync* UTextToSoundWaveAsync::TextToSoundWaveAsync(UObject* WorldContextObject, const FString& SynthesisText, const FAzSpeechSettingsOptions& Options)
{
	UTextToSoundWaveAsync* const NewAsyncTask = NewObject<UTextToSoundWaveAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SynthesisText;
	NewAsyncTask->TaskOptions = GetValidatedOptions(Options);
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

	AsyncTask(ENamedThreads::GameThread,
		[this]
		{
			const TArray<uint8> LastBuffer = GetAudioData();
			SynthesisCompleted.Broadcast(UAzSpeechHelper::ConvertAudioDataToSoundWave(LastBuffer));
		}
	);
}
