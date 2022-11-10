// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToSpeechAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "Async/Async.h"

USSMLToSpeechAsync* USSMLToSpeechAsync::SSMLToSpeech(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToSpeechAsync* const NewAsyncTask = NewObject<USSMLToSpeechAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SSMLString;
	NewAsyncTask->bIsSSMLBased = true;

	return NewAsyncTask;
}

void USSMLToSpeechAsync::StopAzSpeechTask()
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

void USSMLToSpeechAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (SynthesisEventArgs.Result->Reason == Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioCompleted)
	{
		if (CanBroadcastWithReason(SynthesisEventArgs.Result->Reason))
		{
			SynthesisCompleted.Broadcast(IsLastResultValid());
		}

		const TArray<uint8> LastBuffer = GetLastSynthesizedStream();
		if (!UAzSpeechHelper::IsAudioDataValid(LastBuffer))
		{
			return;
		}
		
		AsyncTask(ENamedThreads::GameThread, [this, LastBuffer]
		{
			if (!UAzSpeechTaskBase::IsTaskStillValid(this))
			{
				return;
			}

			AudioComponent = UGameplayStatics::CreateSound2D(WorldContextObject, UAzSpeechHelper::ConvertAudioDataToSoundWave(LastBuffer));
			AudioComponent->Play();
		});
	}
}