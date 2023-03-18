// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/TextToAudioDataAsync.h"
#include <Async/Async.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(TextToAudioDataAsync)
#endif

UTextToAudioDataAsync* UTextToAudioDataAsync::TextToAudioDataAsync(UObject* WorldContextObject, const FString& SynthesisText, const FAzSpeechSettingsOptions& Options)
{
	UTextToAudioDataAsync* const NewAsyncTask = NewObject<UTextToAudioDataAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SynthesisText;
	NewAsyncTask->TaskOptions = GetValidatedOptions(Options);
	NewAsyncTask->bIsSSMLBased = false;
	NewAsyncTask->TaskName = *FString(__func__);
	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}

void UTextToAudioDataAsync::BroadcastFinalResult()
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
			SynthesisCompleted.Broadcast(GetAudioData());
		}
	);
}
