// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToVoiceAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "Async/Async.h"

UTextToVoiceAsync* UTextToVoiceAsync::TextToVoice(const UObject* WorldContextObject, const FString& TextToConvert, const FString& VoiceName, const FString& LanguageId)
{
	UTextToVoiceAsync* const NewAsyncTask = NewObject<UTextToVoiceAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = TextToConvert;
	NewAsyncTask->bIsSSMLBased = false;

	return NewAsyncTask;
}

void UTextToVoiceAsync::StopAzSpeechTask()
{
	Super::StopAzSpeechTask();

	if (AudioComponent.IsValid())
	{
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			AudioComponent->Stop();
			AudioComponent->DestroyComponent();
			AudioComponent.Reset();
		});
	}
}

void UTextToVoiceAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (SynthesisEventArgs.Result->Reason == Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioCompleted)
	{
		const TArray<uint8> LastBuffer = GetLastSynthesizedStream();

#if ENGINE_MAJOR_VERSION >= 5
		if (LastBuffer.IsEmpty())
#else
		if (LastBuffer.Num() == 0)
#endif
		{
			return;
		}

		AsyncTask(ENamedThreads::GameThread, [this, LastBuffer]
		{
			if (!UAzSpeechTaskBase::IsTaskStillValid(this))
			{
				return;
			}

			AudioComponent = UGameplayStatics::CreateSound2D(WorldContextObject, UAzSpeechHelper::ConvertStreamToSoundWave(LastBuffer));
			AudioComponent->Play();
		});
	}
}