// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SSMLToSoundWaveAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include <Sound/SoundWave.h>
#include <Async/Async.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SSMLToSoundWaveAsync)
#endif

USSMLToSoundWaveAsync* USSMLToSoundWaveAsync::SSMLToSoundWave_DefaultOptions(UObject* WorldContextObject, const FString& SynthesisSSML)
{
	return SSMLToSoundWave_CustomOptions(WorldContextObject, SynthesisSSML, FAzSpeechSettingsOptions());
}

USSMLToSoundWaveAsync* USSMLToSoundWaveAsync::SSMLToSoundWave_CustomOptions(UObject* WorldContextObject, const FString& SynthesisSSML, const FAzSpeechSettingsOptions& Options)
{
	USSMLToSoundWaveAsync* const NewAsyncTask = NewObject<USSMLToSoundWaveAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->TaskOptions = GetValidatedOptions(Options);
	NewAsyncTask->SynthesisText = SynthesisSSML;
	NewAsyncTask->bIsSSMLBased = true;
	NewAsyncTask->TaskName = *FString(__func__);
	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}

void USSMLToSoundWaveAsync::BroadcastFinalResult()
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
