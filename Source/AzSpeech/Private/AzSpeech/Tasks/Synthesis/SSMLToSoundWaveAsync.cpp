// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Synthesis/SSMLToSoundWaveAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include <Sound/SoundWave.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SSMLToSoundWaveAsync)
#endif

USSMLToSoundWaveAsync* USSMLToSoundWaveAsync::SSMLToSoundWave_DefaultOptions(UObject* const WorldContextObject, const FString& SynthesisSSML)
{
    return SSMLToSoundWave_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechSynthesisOptions(), SynthesisSSML);
}

USSMLToSoundWaveAsync* USSMLToSoundWaveAsync::SSMLToSoundWave_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML)
{
    USSMLToSoundWaveAsync* const NewAsyncTask = NewObject<USSMLToSoundWaveAsync>();
    NewAsyncTask->WorldContextObject = WorldContextObject;
    NewAsyncTask->SubscriptionOptions = SubscriptionOptions;
    NewAsyncTask->SynthesisOptions = SynthesisOptions;
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
    SynthesisCompleted.Broadcast(UAzSpeechHelper::ConvertAudioDataToSoundWave(GetAudioData()));

    SetReadyToDestroy();
}