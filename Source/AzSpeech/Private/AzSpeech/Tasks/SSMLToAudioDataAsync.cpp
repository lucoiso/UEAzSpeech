// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SSMLToAudioDataAsync.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SSMLToAudioDataAsync)
#endif

USSMLToAudioDataAsync* USSMLToAudioDataAsync::SSMLToAudioData(UObject* WorldContextObject, const FString& SynthesisSSML)
{
	USSMLToAudioDataAsync* const NewAsyncTask = NewObject<USSMLToAudioDataAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SynthesisSSML;
	NewAsyncTask->bIsSSMLBased = true;
	NewAsyncTask->TaskName = *FString(__func__);
	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}

void USSMLToAudioDataAsync::BroadcastFinalResult()
{
	Super::BroadcastFinalResult();

	FScopeLock Lock(&Mutex);

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