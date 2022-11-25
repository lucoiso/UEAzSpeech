// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Bases/AzSpeechSpeechSynthesisBase.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"

void UAzSpeechSpeechSynthesisBase::StopAzSpeechTask()
{
	Super::StopAzSpeechTask();

	if (AudioComponent.IsValid())
	{
		AudioComponent->Stop();
		AudioComponent->DestroyComponent();
		AudioComponent.Reset();
	}
}

void UAzSpeechSpeechSynthesisBase::BroadcastFinalResult()
{
	Super::BroadcastFinalResult();
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

		if (!UAzSpeechTaskBase::IsTaskStillValid(this))
		{
			return;
		}

		SynthesisCompleted.Broadcast(IsLastResultValid());

		// Clear bindings
		BroadcastFinalResult();

		AudioComponent = UGameplayStatics::CreateSound2D(WorldContextObject, UAzSpeechHelper::ConvertAudioDataToSoundWave(LastBuffer));
		AudioComponent->Play();
	}
}