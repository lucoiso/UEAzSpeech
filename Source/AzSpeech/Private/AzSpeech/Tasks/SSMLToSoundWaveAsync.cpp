// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SSMLToSoundWaveAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Sound/SoundWave.h"
#include "Async/Async.h"

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
	
	const TArray<uint8> LastBuffer = GetLastSynthesizedStream();

	if (!UAzSpeechHelper::IsAudioDataValid(LastBuffer))
	{
		return;
	}

	AsyncTask(ENamedThreads::GameThread, [=] { SynthesisCompleted.Broadcast(UAzSpeechHelper::ConvertAudioDataToSoundWave(LastBuffer)); });
}

void USSMLToSoundWaveAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (SynthesisEventArgs.Result->Reason == Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioCompleted)
	{
		BroadcastFinalResult();
	}
}