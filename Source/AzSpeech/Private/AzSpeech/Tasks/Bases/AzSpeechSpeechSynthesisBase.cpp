// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechSpeechSynthesisBase.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"

void UAzSpeechSpeechSynthesisBase::SetReadyToDestroy()
{
	if (IsTaskReadyToDestroy())
	{
		return;
	}

	Super::SetReadyToDestroy();

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

	if (AudioComponent->OnAudioPlayStateChanged.IsBound())
	{
		AudioComponent->OnAudioPlayStateChanged.Clear();
	}
}

void UAzSpeechSpeechSynthesisBase::BroadcastFinalResult()
{
	Super::BroadcastFinalResult();

	if (SynthesisCompleted.IsBound())
	{
		SynthesisCompleted.Broadcast(IsLastResultValid());
		SynthesisCompleted.Clear();
	}
}

void UAzSpeechSpeechSynthesisBase::OnSynthesisUpdate(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult)
{	
	Super::OnSynthesisUpdate(LastResult);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		SetReadyToDestroy();
		return;
	}

	if (CanBroadcastWithReason(LastResult->Reason))
	{
		const TArray<uint8> LastBuffer = GetLastSynthesizedAudioData();
		if (!UAzSpeechHelper::IsAudioDataValid(LastBuffer))
		{
			SetReadyToDestroy();
			return;
		}

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
		SetReadyToDestroy();
	}
}
