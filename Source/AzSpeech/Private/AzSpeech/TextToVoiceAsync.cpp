// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToVoiceAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

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

bool UTextToVoiceAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (TextToConvert.IsEmpty() || VoiceName.IsEmpty() || LanguageID.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Missing parameters"), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: Initializing task"), *FString(__func__));

	const std::string InText = TCHAR_TO_UTF8(*TextToConvert);
	const std::string InLanguage = TCHAR_TO_UTF8(*LanguageID);
	const std::string InVoice = TCHAR_TO_UTF8(*VoiceName);

	const auto AudioConfig = AudioConfig::FromDefaultSpeakerOutput();
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig, InLanguage, InVoice);

	if (!SynthesizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Failed to proceed with task: SynthesizerObject is null"), *FString(__func__));
		return false;
	}

	ApplyExtraSettings();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]
	{
		SynthesizerObject->SpeakTextAsync(InText).wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
	});

	return true;
}

void UTextToVoiceAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (SynthesisEventArgs.Result->Reason != ResultReason::SynthesizingAudio)
	{
		SynthesisCompleted.Broadcast(bLastResultIsValid);
	}
}