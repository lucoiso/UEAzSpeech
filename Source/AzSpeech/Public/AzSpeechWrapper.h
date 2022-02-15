// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "AzSpeechData.h"
#include "Async/Async.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx.h>
THIRD_PARTY_INCLUDES_END

/**
 *
 */
namespace FAzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static wchar_t* DoVoiceToTextWork(const std::string SubscriptionID, const std::string RegionID,
		                                  const std::string LanguageID)
		{
			if (SubscriptionID.size() == 0 || RegionID.size() == 0 || LanguageID.size() == 0)
			{
				return new wchar_t;
			}

			const auto Config = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(
				SubscriptionID, RegionID);
			Config->SetSpeechRecognitionLanguage(LanguageID);
			Config->SetSpeechSynthesisLanguage(LanguageID);
			Config->SetProfanity(Microsoft::CognitiveServices::Speech::ProfanityOption::Raw);

			const auto& SpeechRecognizer = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(Config);
			const auto& SpeechRecognitionResult = SpeechRecognizer->RecognizeOnceAsync().get();

			std::string RecognizedString = "not recognized";

			if (SpeechRecognitionResult->Reason == Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech)
			{
				RecognizedString = SpeechRecognitionResult->Text;
			}

			const std::string Locale = setlocale(LC_ALL, "");
			const char* SrcBuf = RecognizedString.c_str();
			const size_t Size = strlen(SrcBuf) + 1;
			wchar_t* DstBuf = new wchar_t[Size];
			size_t OutSize;
			mbstowcs_s(&OutSize, DstBuf, Size, SrcBuf, Size - 1);
			setlocale(LC_ALL, Locale.c_str());

			return DstBuf;
		}

		static bool DoTextToVoiceWork(const std::string TextToConvert, const std::string SubscriptionID,
		                              const std::string RegionID, const std::string LanguageID,
		                              const std::string VoiceName)
		{
			if (SubscriptionID.size() == 0 || RegionID.size() == 0 || LanguageID.size() == 0 || TextToConvert.size() ==
				0)
			{
				return false;
			}

			const auto SpeechConfig = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(
				SubscriptionID, RegionID);
			SpeechConfig->SetSpeechSynthesisLanguage(LanguageID);
			SpeechConfig->SetSpeechSynthesisVoiceName(VoiceName);
			const auto& SpeechSynthesizer = Microsoft::CognitiveServices::Speech::SpeechSynthesizer::FromConfig(
				SpeechConfig);

			const auto& SpeechSynthesisResult = SpeechSynthesizer->SpeakTextAsync(TextToConvert).get();

			if (SpeechSynthesisResult->Reason ==
				Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioCompleted)
			{
				return true;
			}

			return false;
		}
	} // namespace Standard_Cpp


	namespace Unreal_Cpp
	{
		static void AsyncTextToVoice(const FAzSpeechData Parameters, const FString TextToConvert,
		                             FTextToVoiceDelegate Delegate, const FString VoiceName)
		{
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Parameters, TextToConvert, Delegate, VoiceName]()
			{
				const TFuture<bool> TextToVoiceAsyncWork =
					Async(EAsyncExecution::Thread, [Parameters, TextToConvert, VoiceName]() -> bool
					{
						const std::string SubscriptionID = std::string(TCHAR_TO_UTF8(*Parameters.SubscriptionID));
						const std::string RegionID = std::string(TCHAR_TO_UTF8(*Parameters.RegionID));
						const std::string LanguageID = std::string(TCHAR_TO_UTF8(*Parameters.LanguageID));
						const std::string Name = std::string(TCHAR_TO_UTF8(*VoiceName));
						const std::string ToConvert = std::string(TCHAR_TO_UTF8(*TextToConvert));

						return Standard_Cpp::DoTextToVoiceWork(ToConvert, SubscriptionID, RegionID, LanguageID,
						                                       Name);
					});

				TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(5));
				const bool bOutputValue = TextToVoiceAsyncWork.Get();

				AsyncTask(ENamedThreads::GameThread, [bOutputValue, Delegate]()
				{
					Delegate.Broadcast(bOutputValue);
				});

				const FString OutputValueStr = bOutputValue ? "Success" : "Error";

				UE_LOG(LogTemp, Warning,
				       TEXT("AzSpeech Debug - Subscription: %s, Region: %s, Language: %s, Text To Voice Result: %s"),
				       *FString(Parameters.SubscriptionID), *FString(Parameters.RegionID),
				       *FString(Parameters.LanguageID),
				       *OutputValueStr);
			});
		}


		static void AsyncVoiceToText(const FAzSpeechData Parameters, FVoiceToTextDelegate Delegate)
		{
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Parameters, Delegate]()
			{
				const TFuture<wchar_t*> VoiceToTextAsyncWork =
					Async(EAsyncExecution::Thread, [Parameters]() -> wchar_t*
					{
						const std::string SubscriptionID = std::string(TCHAR_TO_UTF8(*Parameters.SubscriptionID));
						const std::string RegionID = std::string(TCHAR_TO_UTF8(*Parameters.RegionID));
						const std::string LanguageID = std::string(TCHAR_TO_UTF8(*Parameters.LanguageID));

						return Standard_Cpp::DoVoiceToTextWork(SubscriptionID, RegionID, LanguageID);
					});

				VoiceToTextAsyncWork.WaitFor(FTimespan::FromSeconds(5));
				const FString& RecognizedString = VoiceToTextAsyncWork.Get();

				AsyncTask(ENamedThreads::GameThread, [RecognizedString, Delegate]()
				{
					Delegate.Broadcast(RecognizedString);
				});

				UE_LOG(LogTemp, Warning,
				       TEXT("AzSpeech Debug - Subscription: %s, Region: %s, Language: %s, Voice To Text Result: %s"),
				       *FString(Parameters.SubscriptionID), *FString(Parameters.RegionID),
				       *FString(Parameters.LanguageID),
				       *RecognizedString);
			});
		}
	} // namespace Unreal_Cpp
} // namespace FAzSpeechWrapper
