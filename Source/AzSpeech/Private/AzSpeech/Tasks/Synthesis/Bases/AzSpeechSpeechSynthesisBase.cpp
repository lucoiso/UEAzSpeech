// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Synthesis/Bases/AzSpeechSpeechSynthesisBase.h"
#include "AzSpeech/Structures/AzSpeechTaskData.h"
#include "AzSpeech/AzSpeechHelper.h"
#include <Components/AudioComponent.h>
#include <Kismet/GameplayStatics.h>
#include <Sound/SoundWave.h>
#include <Async/Async.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechSpeechSynthesisBase)
#endif

void UAzSpeechSpeechSynthesisBase::StopAzSpeechTask()
{
    Super::StopAzSpeechTask();

    if (!AudioComponent.IsValid())
    {
        return;
    }

    if (AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }

    AudioComponent->DestroyComponent();
    AudioComponent.Reset();
}

void UAzSpeechSpeechSynthesisBase::SetReadyToDestroy()
{
    // Only set as ready to destroy after the sound stop playing normally or the user ask to stop
    if (AudioComponent.IsValid() && AudioComponent->IsPlaying())
    {
        return;
    }

    Super::SetReadyToDestroy();
}

void UAzSpeechSpeechSynthesisBase::BroadcastFinalResult()
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
            if (bAutoPlayAudio)
            {
                PlayAudio();
            }

            SynthesisCompleted.Broadcast(IsLastResultValid());
        }
    );
}

void UAzSpeechSpeechSynthesisBase::OnAudioPlayStateChanged(const EAudioComponentPlayState PlayState)
{
    FScopeLock Lock(&Mutex);

    if (!UAzSpeechTaskStatus::IsTaskStillValid(this))
    {
        return;
    }

    if (PlayState == EAudioComponentPlayState::Stopped)
    {
        InternalAudioFinished.ExecuteIfBound(FAzSpeechTaskData{ GetUniqueID(), GetClass() });
        InternalAudioFinished.Unbind();

        SetReadyToDestroy();
    }
}

void UAzSpeechSpeechSynthesisBase::PlayAudio()
{
    check(IsInGameThread());

    AudioComponent = UGameplayStatics::CreateSound2D(WorldContextObject.Get(), UAzSpeechHelper::ConvertAudioDataToSoundWave(GetAudioData()));

    if (!AudioComponent.IsValid())
    {
        SetReadyToDestroy();
        return;
    }

    FScriptDelegate UniqueDelegate_AudioStateChanged;
    UniqueDelegate_AudioStateChanged.BindUFunction(this, TEXT("OnAudioPlayStateChanged"));
    AudioComponent->OnAudioPlayStateChanged.AddUnique(UniqueDelegate_AudioStateChanged);

    AudioComponent->Play();
}