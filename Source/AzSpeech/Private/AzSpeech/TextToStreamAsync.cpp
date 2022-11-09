// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToStreamAsync.h"
#include "AzSpeechInternalFuncs.h"
#include "Async/Async.h"

UTextToStreamAsync* UTextToStreamAsync::TextToStream(const UObject* WorldContextObject, const FString& TextToConvert, const FString& VoiceName, const FString& LanguageId)
{
	UTextToStreamAsync* const NewAsyncTask = NewObject<UTextToStreamAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = TextToConvert;
	NewAsyncTask->bIsSSMLBased = false;

	return NewAsyncTask;
}

bool UTextToStreamAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (HasEmptyParam(SynthesisText, VoiceName, LanguageId))
	{
		return false;
	}

	const auto AudioConfig = Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromStreamOutput(Microsoft::CognitiveServices::Speech::Audio::AudioOutputStream::CreatePullStream());
	if (!InitializeSynthesizer(AudioConfig))
	{
		return false;
	}

	StartSynthesisWork();
	return true;
}

void UTextToStreamAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(SynthesisEventArgs.Result->Reason))
	{
		const TArray<uint8> OutputStream = GetLastSynthesizedStream();

#if ENGINE_MAJOR_VERSION >= 5
		OutputSynthesisResult(!OutputStream.IsEmpty());
#else
		OutputSynthesisResult(OutputStream.Num() != 0);
#endif

		AsyncTask(ENamedThreads::GameThread, [=] { SynthesisCompleted.Broadcast(OutputStream); });
	}
}