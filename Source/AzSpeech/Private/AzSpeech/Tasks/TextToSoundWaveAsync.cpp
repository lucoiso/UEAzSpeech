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

	if (SynthesisCompleted.IsBound())
	{
		const TArray<uint8> LastBuffer = GetLastSynthesizedAudioData();
		SynthesisCompleted.Broadcast(UAzSpeechHelper::ConvertAudioDataToSoundWave(LastBuffer));
		SynthesisCompleted.Clear();
	}
}

void UTextToSoundWaveAsync::OnSynthesisUpdate(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult)
{
	Super::OnSynthesisUpdate(LastResult);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(LastResult->Reason))
	{
		BroadcastFinalResult();
	}
}