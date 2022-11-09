// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToStreamAsync.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

UTextToStreamAsync* UTextToStreamAsync::TextToStream(const UObject* WorldContextObject, const FString& TextToConvert, const FString& VoiceName, const FString& LanguageId)
{
	UTextToStreamAsync* const NewAsyncTask = NewObject<UTextToStreamAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->TextToConvert = TextToConvert;
	NewAsyncTask->VoiceName = AzSpeech::Internal::GetVoiceName(VoiceName);
	NewAsyncTask->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);

	return NewAsyncTask;
}

void UTextToStreamAsync::Activate()
{
	Super::Activate();
}

bool UTextToStreamAsync::StartAzureTaskWork_Internal()
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

	const auto AudioConfig = AudioConfig::FromStreamOutput(AudioOutputStream::CreatePullStream());
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

void UTextToStreamAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (SynthesisEventArgs.Result->Reason != ResultReason::SynthesizingAudio)
	{
		const TArray<uint8> OutputStream = GetLastSynthesizedStream();

		if (SynthesisEventArgs.Result->Reason != ResultReason::SynthesizingAudioStarted)
		{
#if ENGINE_MAJOR_VERSION >= 5
			OutputSynthesisResult(!OutputStream.IsEmpty());
#else
			OutputSynthesisResult(OutputStream.Num() != 0);
#endif
		}

		SynthesisCompleted.Broadcast(OutputStream);
	}
}