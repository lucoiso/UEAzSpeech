// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/TextToSoundWaveAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Sound/SoundWave.h"
#include "Async/Async.h"

UTextToSoundWaveAsync* UTextToSoundWaveAsync::TextToSoundWave(const UObject* WorldContextObject, const FString& TextToConvert, const FString& VoiceName, const FString& LanguageId)
{
	UTextToSoundWaveAsync* const NewAsyncTask = NewObject<UTextToSoundWaveAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = TextToConvert;
	NewAsyncTask->VoiceName = VoiceName;
	NewAsyncTask->LanguageId = LanguageId;
	NewAsyncTask->bIsSSMLBased = false;

	return NewAsyncTask;
}

void UTextToSoundWaveAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
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