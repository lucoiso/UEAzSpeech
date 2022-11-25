// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SSMLToAudioDataAsync.h"

USSMLToAudioDataAsync* USSMLToAudioDataAsync::SSMLToAudioData(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToAudioDataAsync* const NewAsyncTask = NewObject<USSMLToAudioDataAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SSMLString;
	NewAsyncTask->bIsSSMLBased = true;
	NewAsyncTask->TaskName = *FString(__func__);

	return NewAsyncTask;
}

void USSMLToAudioDataAsync::BroadcastFinalResult()
{	
	SynthesisCompleted.Broadcast(GetLastSynthesizedAudioData());
	Super::BroadcastFinalResult();
}

void USSMLToAudioDataAsync::OnSynthesisUpdate()
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