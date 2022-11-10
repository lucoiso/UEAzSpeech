// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToSoundWaveAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Sound/SoundWave.h"
#include "Async/Async.h"

USSMLToSoundWaveAsync* USSMLToSoundWaveAsync::SSMLToSoundWave(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToSoundWaveAsync* const NewAsyncTask = NewObject<USSMLToSoundWaveAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SSMLString;
	NewAsyncTask->bIsSSMLBased = true;

	return NewAsyncTask;
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
		const TArray<uint8> LastBuffer = GetLastSynthesizedStream();

		if (!UAzSpeechHelper::IsAudioDataValid(LastBuffer))
		{
			return;
		}
		
		AsyncTask(ENamedThreads::GameThread, [=] { SynthesisCompleted.Broadcast(UAzSpeechHelper::ConvertAudioDataToSoundWave(LastBuffer)); });
	}
}