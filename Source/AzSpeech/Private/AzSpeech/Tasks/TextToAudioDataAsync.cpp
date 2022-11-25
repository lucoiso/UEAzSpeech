// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/TextToAudioDataAsync.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"

UTextToAudioDataAsync* UTextToAudioDataAsync::TextToAudioData(const UObject* WorldContextObject, const FString& TextToConvert, const FString& VoiceName, const FString& LanguageId)
{
	UTextToAudioDataAsync* const NewAsyncTask = NewObject<UTextToAudioDataAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = TextToConvert;
	NewAsyncTask->VoiceName = VoiceName;
	NewAsyncTask->LanguageId = LanguageId;
	NewAsyncTask->bIsSSMLBased = false;
	NewAsyncTask->TaskName = *FString(__func__);

	return NewAsyncTask;
}

void UTextToAudioDataAsync::BroadcastFinalResult()
{
	FScopeLock Lock(&Mutex);
	
	SynthesisCompleted.Broadcast(GetLastSynthesizedAudioData());
	Super::BroadcastFinalResult();
}

void UTextToAudioDataAsync::OnSynthesisUpdate()
{
	Super::OnSynthesisUpdate();

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(LastSynthesisResult->Reason))
	{
		FScopeLock Lock(&Mutex);
		BroadcastFinalResult();
	}
}