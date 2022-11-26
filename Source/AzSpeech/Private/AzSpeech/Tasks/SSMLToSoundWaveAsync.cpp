// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SSMLToSoundWaveAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Sound/SoundWave.h"

USSMLToSoundWaveAsync* USSMLToSoundWaveAsync::SSMLToSoundWave(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToSoundWaveAsync* const NewAsyncTask = NewObject<USSMLToSoundWaveAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SSMLString;
	NewAsyncTask->bIsSSMLBased = true;
	NewAsyncTask->TaskName = *FString(__func__);

	return NewAsyncTask;
}

void USSMLToSoundWaveAsync::BroadcastFinalResult()
{
	Super::BroadcastFinalResult();

	if (SynthesisCompleted.IsBound())
	{
		const TArray<uint8> LastBuffer = GetLastSynthesizedAudioData();
		SynthesisCompleted.Broadcast(UAzSpeechHelper::ConvertAudioDataToSoundWave(LastBuffer));
	}

	SetReadyToDestroy();
}

void USSMLToSoundWaveAsync::OnSynthesisUpdate()
{
	Super::OnSynthesisUpdate();

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(LastSynthesisResult->Reason))
	{
		BroadcastFinalResult();
	}
}