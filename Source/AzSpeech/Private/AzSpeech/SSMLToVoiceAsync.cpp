// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToVoiceAsync.h"
#include "AzSpeechInternalFuncs.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "Async/Async.h"

USSMLToVoiceAsync* USSMLToVoiceAsync::SSMLToVoice(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToVoiceAsync* const NewAsyncTask = NewObject<USSMLToVoiceAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SSMLString = SSMLString;

	return NewAsyncTask;
}

void USSMLToVoiceAsync::Activate()
{
	Super::Activate();
}

void USSMLToVoiceAsync::StopAzSpeechTask()
{	
	if (AudioComponent.IsValid())
	{
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			AudioComponent->Stop();
			AudioComponent->DestroyComponent();
			AudioComponent.Reset();
		});
	}
	
	Super::StopAzSpeechTask();
}

void USSMLToVoiceAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (SynthesisEventArgs.Result->Reason == ResultReason::SynthesizingAudioCompleted)
	{
		const TArray<uint8> LastBuffer = GetLastSynthesizedStream();

		if (LastBuffer.IsEmpty())
		{
			return;
		}
		
		AsyncTask(ENamedThreads::GameThread, [this, LastBuffer]
		{
			AudioComponent = UGameplayStatics::CreateSound2D(WorldContextObject, UAzSpeechHelper::ConvertStreamToSoundWave(LastBuffer));
			AudioComponent->Play();
		});
	}
}