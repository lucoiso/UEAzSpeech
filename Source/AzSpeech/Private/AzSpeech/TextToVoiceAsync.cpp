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
			const auto AudioConfig = AudioConfig::FromDefaultSpeakerOutput();
			const auto Synthesizer = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig, InLanguageID, InVoiceName);

			if (Synthesizer == nullptr)
			{
				return false;
			}

			const auto SynthesisResult = Synthesizer->SpeakTextAsync(InStr).get();

			return AzSpeech::Internal::ProcessAzSpeechResult(SynthesisResult->Reason);
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncTextToVoice(const FString& InStr, const FString& InVoiceName, const FString& InLanguageID, const FTextToVoiceDelegate& InDelegate)
		{
			if (InStr.IsEmpty() || InVoiceName.IsEmpty() || InLanguageID.IsEmpty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, InStr, InVoiceName, InLanguageID, InDelegate]
			{
				const TFuture<bool> TextToVoiceAsyncWork = Async(EAsyncExecution::Thread, [=]() -> bool
				{
					const std::string InLanguageIDStr = TCHAR_TO_UTF8(*InLanguageID);
					const std::string InVoiceNameStr = TCHAR_TO_UTF8(*InVoiceName);
					const std::string InConvertStr = TCHAR_TO_UTF8(*InStr);

					return Standard_Cpp::DoTextToVoiceWork(InConvertStr, InLanguageIDStr, InVoiceNameStr);
				});

				if (!TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
					return;
				}

				const bool bOutputValue = TextToVoiceAsyncWork.Get();

				InDelegate.Broadcast(bOutputValue);

				if (bOutputValue)
				{
					UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(FuncName));
				}
				else
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Error"), *FString(FuncName));
				}
			});
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
	AzSpeechWrapper::Unreal_Cpp::AsyncTextToVoice(TextToConvert, VoiceName, LanguageID, TaskCompleted);
}
