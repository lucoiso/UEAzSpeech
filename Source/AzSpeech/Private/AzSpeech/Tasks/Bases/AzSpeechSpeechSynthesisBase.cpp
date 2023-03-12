// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechSpeechSynthesisBase.h"
#include "AzSpeech/AzSpeechHelper.h"
#include <Kismet/GameplayStatics.h>
#include <Sound/SoundWave.h>

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

	SynthesisCompleted.Broadcast(IsLastResultValid());

	AudioComponent = UGameplayStatics::CreateSound2D(WorldContextObject, UAzSpeechHelper::ConvertAudioDataToSoundWave(GetAudioData()));

	FScriptDelegate UniqueDelegate_AudioStateChanged;
	UniqueDelegate_AudioStateChanged.BindUFunction(this, TEXT("OnAudioPlayStateChanged"));
	AudioComponent->OnAudioPlayStateChanged.AddUnique(UniqueDelegate_AudioStateChanged);

	AudioComponent->Play();
}

void UAzSpeechSpeechSynthesisBase::OnAudioPlayStateChanged(const EAudioComponentPlayState PlayState)
{
	if (!UAzSpeechTaskStatus::IsTaskStillValid(this))
	{
		return;
	}

	AudioComponent->OnAudioPlayStateChanged.Clear();

	if (PlayState == EAudioComponentPlayState::Stopped)
	{
		SetReadyToDestroy();
	}
}
