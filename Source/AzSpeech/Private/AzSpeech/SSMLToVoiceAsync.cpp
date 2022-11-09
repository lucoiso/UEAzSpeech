// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToVoiceAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "Async/Async.h"

USSMLToVoiceAsync* USSMLToVoiceAsync::SSMLToVoice(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToVoiceAsync* const NewAsyncTask = NewObject<USSMLToVoiceAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SSMLString;
	NewAsyncTask->bIsSSMLBased = true;

	return NewAsyncTask;
}

void USSMLToVoiceAsync::StopAzSpeechTask()
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

void USSMLToVoiceAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
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