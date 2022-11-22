// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SSMLToAudioDataAsync.h"
#include "Async/Async.h"

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
	Super::BroadcastFinalResult();
	AsyncTask(ENamedThreads::GameThread, [=] { SynthesisCompleted.Broadcast(GetLastSynthesizedStream()); });
}

void USSMLToAudioDataAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(SynthesisEventArgs.Result->Reason))
	{
		BroadcastFinalResult();
	}
}