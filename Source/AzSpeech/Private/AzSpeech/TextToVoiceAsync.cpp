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
		static bool DoTextToVoiceWork(const std::string& InStr,
		                              const std::string& InLanguageID,
		                              const std::string& InVoiceName)
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
		static void AsyncTextToVoice(const FString& InStr,
		                             const FString& InVoiceName,
		                             const FString& InLanguageID,
		                             const FTextToVoiceDelegate& InDelegate)
		{
			if (InStr.IsEmpty() || InVoiceName.IsEmpty() || InLanguageID.IsEmpty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [InStr, InDelegate, InVoiceName, InLanguageID]
			{
				const TFuture<bool> TextToVoiceAsyncWork =
					Async(EAsyncExecution::Thread, [InStr, InVoiceName, InLanguageID]() -> bool
					{
						const std::string InLanguageIDStr = TCHAR_TO_UTF8(*InLanguageID);
						const std::string InVoiceNameStr = TCHAR_TO_UTF8(*InVoiceName);
						const std::string InConvertStr = TCHAR_TO_UTF8(*InStr);

						return Standard_Cpp::DoTextToVoiceWork(InConvertStr, InLanguageIDStr, InVoiceNameStr);
					});

				if (!TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(15)))
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncTextToVoice: Task timed out"));
					return;
				}
				
				const bool bOutputValue = TextToVoiceAsyncWork.Get();

				AsyncTask(ENamedThreads::GameThread, [bOutputValue, InDelegate]
				{
					InDelegate.Broadcast(bOutputValue);
				});

				if (bOutputValue)
				{
					UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - AsyncTextToVoice: Result: Success"));
				}
				else
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncTextToVoice: Result: Error"));
				}
			});
		}
	}
}

UTextToVoiceAsync* UTextToVoiceAsync::TextToVoice(const UObject* WorldContextObject,
                                                  const FString& TextToConvert,
                                                  const FString& VoiceName,
                                                  const FString& LanguageId)
{
	UTextToVoiceAsync* TextToVoiceAsync = NewObject<UTextToVoiceAsync>();
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
