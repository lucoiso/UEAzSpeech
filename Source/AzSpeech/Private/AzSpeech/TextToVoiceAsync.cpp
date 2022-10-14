// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToVoiceAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static bool DoTextToVoiceWork(const std::string& InStr, const std::string& InLanguageID, const std::string& InVoiceName)
		{
			
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncTextToVoice(const FString& InStr, const FString& InVoiceName, const FString& InLanguageID, const FTextToVoiceDelegate& InDelegate)
		{
			
		}
	}
}

UTextToVoiceAsync* UTextToVoiceAsync::TextToVoice(const UObject* WorldContextObject, const FString& TextToConvert, const FString& VoiceName, const FString& LanguageId)
{
	UTextToVoiceAsync* const TextToVoiceAsync = NewObject<UTextToVoiceAsync>();
	TextToVoiceAsync->WorldContextObject = WorldContextObject;
	TextToVoiceAsync->TextToConvert = TextToConvert;
	TextToVoiceAsync->VoiceName = AzSpeech::Internal::GetVoiceName(VoiceName);
	TextToVoiceAsync->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);

	return TextToVoiceAsync;
}

void UTextToVoiceAsync::Activate()
{
	StartAzureTaskWork_Internal();
}

void UTextToVoiceAsync::StartAzureTaskWork_Internal()
{
	if (TextToConvert.IsEmpty() || VoiceName.IsEmpty() || LanguageID.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

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
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
			return;
		}

		const bool bOutputValue = TextToVoiceAsyncWork.Get();
		AsyncTask(ENamedThreads::GameThread, [=]() { TaskCompleted.Broadcast(bOutputValue); });

		if (bOutputValue)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(FuncName));
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed"), *FString(FuncName));
		}
	});
}

bool UTextToVoiceAsync::DoAzureTaskWork_Internal(const std::string& InStr, const std::string& InLanguageID, const std::string& InVoiceName)
{
	const auto AudioConfig = AudioConfig::FromDefaultSpeakerOutput();
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig, InLanguageID, InVoiceName);

	if (!SynthesizerObject)
	{
		return false;
	}

	const auto SynthesisResult = SynthesizerObject->SpeakTextAsync(InStr).get();

	return AzSpeech::Internal::ProcessSynthesizResult(SynthesisResult);
}
