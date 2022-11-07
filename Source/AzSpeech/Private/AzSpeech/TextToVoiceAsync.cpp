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

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, this]
	{
		const TFuture<bool> TextToVoiceAsyncWork = Async(EAsyncExecution::Thread, [=]() -> bool
		{
			const std::string InLanguageIDStr = TCHAR_TO_UTF8(*LanguageID);
			const std::string InVoiceNameStr = TCHAR_TO_UTF8(*VoiceName);
			const std::string InConvertStr = TCHAR_TO_UTF8(*TextToConvert);

			return DoAzureTaskWork_Internal(InConvertStr, InLanguageIDStr, InVoiceNameStr);
		});

		if (!TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Task timed out"), *FString(FuncName));
			return;
		}

		const bool bOutputValue = TextToVoiceAsyncWork.Get();
		if (CanBroadcast())
		{
			AsyncTask(ENamedThreads::GameThread, [=]() { SynthesisCompleted.Broadcast(bOutputValue); });
		}

		if (bOutputValue)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("%s: Result: Success"), *FString(FuncName));
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Result: Failed"), *FString(FuncName));
		}
	});

	return true;
}

bool UTextToVoiceAsync::DoAzureTaskWork_Internal(const std::string& InStr, const std::string& InLanguageID, const std::string& InVoiceName)
{
	const auto AudioConfig = AudioConfig::FromDefaultSpeakerOutput();
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig, InLanguageID, InVoiceName);

	if (!SynthesizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Failed to proceed with task: SynthesizerObject is null"), *FString(__func__));
		return false;
	}

	EnableVisemeOutput();

	const auto SynthesisResult = SynthesizerObject->StartSpeakingTextAsync(InStr).get();

	return AzSpeech::Internal::ProcessSynthesisResult(SynthesisResult);
}
