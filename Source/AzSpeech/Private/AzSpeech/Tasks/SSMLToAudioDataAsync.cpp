// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SSMLToAudioDataAsync.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SSMLToAudioDataAsync)
#endif

USSMLToAudioDataAsync* USSMLToAudioDataAsync::SSMLToAudioData_DefaultOptions(UObject* WorldContextObject, const FString& SynthesisSSML)
{
	return SSMLToAudioData_CustomOptions(WorldContextObject, SynthesisSSML, FAzSpeechSettingsOptions());
}

USSMLToAudioDataAsync* USSMLToAudioDataAsync::SSMLToAudioData_CustomOptions(UObject* WorldContextObject, const FString& SynthesisSSML, const FAzSpeechSettingsOptions& Options)
{
	USSMLToAudioDataAsync* const NewAsyncTask = NewObject<USSMLToAudioDataAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->TaskOptions = GetValidatedOptions(Options);
	NewAsyncTask->SynthesisText = SynthesisSSML;
	NewAsyncTask->bIsSSMLBased = true;
	NewAsyncTask->TaskName = *FString(__func__);
	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}

void USSMLToAudioDataAsync::BroadcastFinalResult()
{
	FScopeLock Lock(&Mutex);

	if (!UAzSpeechTaskStatus::IsTaskActive(this))
	{
		return;
	}

	Super::BroadcastFinalResult();

	SynthesisCompleted.Broadcast(GetAudioData());
}
