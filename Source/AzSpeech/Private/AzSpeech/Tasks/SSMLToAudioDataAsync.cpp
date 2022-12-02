// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SSMLToAudioDataAsync.h"

USSMLToAudioDataAsync* USSMLToAudioDataAsync::SSMLToAudioData(const UObject* WorldContextObject, const FString& SynthesisSSML)
{
	USSMLToAudioDataAsync* const NewAsyncTask = NewObject<USSMLToAudioDataAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SynthesisSSML;
	NewAsyncTask->bIsSSMLBased = true;
	NewAsyncTask->TaskName = *FString(__func__);

	return NewAsyncTask;
}

void USSMLToAudioDataAsync::BroadcastFinalResult()
{
	Super::BroadcastFinalResult();

	if (SynthesisCompleted.IsBound())
	{
		SynthesisCompleted.Broadcast(GetAudioData());
		SynthesisCompleted.Clear();
	}
}

void USSMLToAudioDataAsync::OnSynthesisUpdate(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult)
{
	Super::OnSynthesisUpdate(LastResult);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(LastResult->Reason))
	{
		FScopeLock Lock(&Mutex);

		BroadcastFinalResult();
	}
}