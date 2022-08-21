// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToStreamAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static std::vector<uint8_t> DoTextToStreamWork(const std::string& InStr,
		                                               const std::string& InLanguageID,
		                                               const std::string& InVoiceName)
		{
			const auto& AudioConfig = AudioConfig::FromStreamOutput(AudioOutputStream::CreatePullStream());
			const auto& SpeechSynthesizer =
				AzSpeech::Internal::GetAzureSynthesizer(AudioConfig, InLanguageID, InVoiceName);

			if (const auto& SpeechSynthesisResult = SpeechSynthesizer->SpeakTextAsync(InStr).get();
				AzSpeech::Internal::ProcessAzSpeechResult(SpeechSynthesisResult->Reason))
			{
				return *SpeechSynthesisResult->GetAudioData().get();
			}

			return std::vector<uint8_t>();
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncTextToStream(const FString& InStr,
		                              const FString& InVoiceName,
		                              const FString& InLanguageID,
		                              FTextToStreamDelegate InDelegate)
		{
			if (InStr.IsEmpty() || InVoiceName.IsEmpty() || InLanguageID.IsEmpty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [InStr, InVoiceName, InLanguageID, InDelegate]
			{
				const TFuture<std::vector<uint8_t>>& TextToVoiceAsyncWork =
					Async(EAsyncExecution::Thread, [InStr, InVoiceName, InLanguageID]() -> std::vector<uint8_t>
					{
						const std::string& InLanguageIDStr = TCHAR_TO_UTF8(*InLanguageID);
						const std::string& InNameIDStr = TCHAR_TO_UTF8(*InVoiceName);
						const std::string& InConvertStr = TCHAR_TO_UTF8(*InStr);

						return Standard_Cpp::DoTextToStreamWork(InConvertStr, InLanguageIDStr, InNameIDStr);
					});

				TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(5));

				const std::vector<uint8_t>& Result = TextToVoiceAsyncWork.Get();
				const bool& bOutputValue = !Result.empty();

				TArray<uint8> OutputArr;
				for (const uint8_t i : Result)
				{
					OutputArr.Add(static_cast<uint8>(i));
				}

				AsyncTask(ENamedThreads::GameThread, [OutputArr, InDelegate]
				{
					InDelegate.Broadcast(OutputArr);
				});

				if (bOutputValue)
				{
					UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - AsyncTextToStream: Result: Success"));
				}
				else
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncTextToStream: Result: Error"));
				}
			});
		}
	}
}

UTextToStreamAsync* UTextToStreamAsync::TextToStream(const UObject* WorldContextObject,
                                                     const FString& TextToConvert,
                                                     const FString& VoiceName,
                                                     const FString& LanguageId)
{
	UTextToStreamAsync* NewAsyncTask = NewObject<UTextToStreamAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->TextToConvert = TextToConvert;
	NewAsyncTask->VoiceName = AzSpeech::Internal::GetVoiceName(VoiceName);
	NewAsyncTask->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);

	return NewAsyncTask;
}

void UTextToStreamAsync::Activate()
{
	AzSpeechWrapper::Unreal_Cpp::AsyncTextToStream(TextToConvert, VoiceName, LanguageID, TaskCompleted);
}
