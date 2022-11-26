// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/TextToSoundWaveAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Sound/SoundWave.h"

UTextToSoundWaveAsync* UTextToSoundWaveAsync::TextToSoundWave(const UObject* WorldContextObject, const FString& TextToConvert, const FString& VoiceName, const FString& LanguageId)
{
	UTextToSoundWaveAsync* const NewAsyncTask = NewObject<UTextToSoundWaveAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = TextToConvert;
	NewAsyncTask->VoiceName = VoiceName;
	NewAsyncTask->LanguageId = LanguageId;
	NewAsyncTask->bIsSSMLBased = false;
	NewAsyncTask->TaskName = *FString(__func__);

	return NewAsyncTask;
}

void UTextToSoundWaveAsync::BroadcastFinalResult()
{
	Super::BroadcastFinalResult();

	const TArray<uint8> LastBuffer = GetLastSynthesizedAudioData();
	if (!UAzSpeechHelper::IsAudioDataValid(LastBuffer))
	{
		return;
	}

	SynthesisCompleted.Broadcast(UAzSpeechHelper::ConvertAudioDataToSoundWave(LastBuffer));
	SetReadyToDestroy();
}

void UTextToSoundWaveAsync::OnSynthesisUpdate()
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