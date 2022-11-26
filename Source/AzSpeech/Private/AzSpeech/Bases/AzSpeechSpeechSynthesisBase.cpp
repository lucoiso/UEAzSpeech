// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Bases/AzSpeechSpeechSynthesisBase.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"

void UAzSpeechSpeechSynthesisBase::StopAzSpeechTask()
{
	Super::StopAzSpeechTask();

	if (!AudioComponent.IsValid())
	{
		return;
	}

	if (AudioComponent->OnAudioPlayStateChanged.IsBound())
	{
		AudioComponent->OnAudioPlayStateChanged.RemoveAll(this);
	}

	if (AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}

	AudioComponent->DestroyComponent();
	AudioComponent.Reset();
}

void UAzSpeechSpeechSynthesisBase::OnSynthesisUpdate()
{	
	Super::OnSynthesisUpdate();

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(LastSynthesisResult->Reason))
	{
		const TArray<uint8> LastBuffer = GetLastSynthesizedAudioData();
		if (!UAzSpeechHelper::IsAudioDataValid(LastBuffer))
		{
			return;
		}

		SynthesisCompleted.Broadcast(IsLastResultValid());

		// Clear bindings
		BroadcastFinalResult();

		AudioComponent = UGameplayStatics::CreateSound2D(WorldContextObject, UAzSpeechHelper::ConvertAudioDataToSoundWave(LastBuffer));

		FScriptDelegate UniqueDelegate_AudioStateChanged;
		UniqueDelegate_AudioStateChanged.BindUFunction(this, TEXT("OnAudioPlayStateChanged"));
		AudioComponent->OnAudioPlayStateChanged.AddUnique(UniqueDelegate_AudioStateChanged);

		AudioComponent->Play();
	}
}

void UAzSpeechSpeechSynthesisBase::OnAudioPlayStateChanged(const EAudioComponentPlayState PlayState)
{
	if (!IsTaskStillValid(this))
	{
		return;
	}

	if (PlayState == EAudioComponentPlayState::Stopped)
	{
		StopAzSpeechTask();
	}
}
