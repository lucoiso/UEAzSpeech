// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToVoiceAsync.h"
#include "AzSpeechInternalFuncs.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "Async/Async.h"

UTextToVoiceAsync* UTextToVoiceAsync::TextToVoice(const UObject* WorldContextObject, const FString& TextToConvert, const FString& VoiceName, const FString& LanguageId)
{
	UTextToVoiceAsync* const NewAsyncTask = NewObject<UTextToVoiceAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->TextToConvert = TextToConvert;
	NewAsyncTask->VoiceName = AzSpeech::Internal::GetVoiceName(VoiceName);
	NewAsyncTask->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);

	return NewAsyncTask;
}

void UTextToVoiceAsync::Activate()
{
	Super::Activate();
}

void UTextToVoiceAsync::StopAzSpeechTask()
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

void UTextToVoiceAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
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