// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToStreamAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

USSMLToStreamAsync* USSMLToStreamAsync::SSMLToStream(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToStreamAsync* const NewAsyncTask = NewObject<USSMLToStreamAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SSMLString = SSMLString;

	return NewAsyncTask;
}

void USSMLToStreamAsync::Activate()
{
	Super::Activate();
}

bool USSMLToStreamAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (SSMLString.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: SSML is empty"), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: Initializing task"), *FString(__func__));

	const std::string InSSML = TCHAR_TO_UTF8(*SSMLString);

	const auto AudioConfig = AudioConfig::FromStreamOutput(AudioOutputStream::CreatePullStream());
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig);

	if (!SynthesizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Failed to proceed with task: SynthesizerObject is null"), *FString(__func__));
		return false;
	}

	ApplyExtraSettings();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]
	{
		SynthesizerObject->SpeakSsmlAsync(InSSML).wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
	});

	return true;
}

void USSMLToStreamAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (SynthesisEventArgs.Result->Reason == ResultReason::SynthesizingAudioCompleted)
	{
		SynthesisCompleted.Broadcast(GetUnrealStreamResult(*SynthesisEventArgs.Result->GetAudioData().get()));
	}
}