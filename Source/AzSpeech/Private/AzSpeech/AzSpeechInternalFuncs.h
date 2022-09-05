// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once
#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechSettings.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx.h>
THIRD_PARTY_INCLUDES_END

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

namespace AzSpeech::Internal
{
	static std::map<int, std::string> GetAzSpeechKeys()
	{
		std::map<int, std::string> Output;
		const UAzSpeechSettings* Settings = GetDefault<UAzSpeechSettings>();

		const auto UpdateSettingsMap = [&Output](const int& InId, const FString& InString)
		{
			const std::string InStr = TCHAR_TO_UTF8(*InString);
			Output.insert(std::make_pair(InId, InStr));
		};

		UpdateSettingsMap(0, Settings->APIAccessKey);
		UpdateSettingsMap(1, Settings->RegionID);
		UpdateSettingsMap(2, Settings->LanguageID);
		UpdateSettingsMap(3, Settings->VoiceName);

		return Output;
	}

	static std::vector<std::string> GetCandidateLanguages()
	{
		std::vector<std::string> Output;

		const UAzSpeechSettings* Settings = GetDefault<UAzSpeechSettings>();
		for (const FString& Iterator : Settings->AutoLanguageCandidates)
		{
			if (Iterator.IsEmpty())
			{
				continue;
			}
			
			Output.push_back(TCHAR_TO_UTF8(*Iterator));
		}

		return Output;
	}

	static FString GetLanguageID(const FString& InTestId = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		if (InTestId.IsEmpty() || InTestId.Equals("Default", ESearchCase::IgnoreCase))
		{
			return UTF8_TO_TCHAR(Settings.at(2).c_str());
		}

		return InTestId;
	}

	static FString GetVoiceName(const FString& InTestId = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		if (InTestId.IsEmpty() || InTestId.Equals("Default", ESearchCase::IgnoreCase))
		{
			return UTF8_TO_TCHAR(Settings.at(3).c_str());
		}

		return InTestId;
	}

	static std::shared_ptr<SpeechSynthesizer> GetAzureSynthesizer(const std::shared_ptr<AudioConfig>& InAudioConfig = AudioConfig::FromDefaultSpeakerOutput(),
																  const std::string& InLanguage = "Default",
																  const std::string& InVoiceName = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		const auto SpeechConfig = SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));

		if (FString(UTF8_TO_TCHAR(InLanguage.c_str())).Equals("Auto", ESearchCase::IgnoreCase))
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing language auto detection"), *FString(__func__));

			UE_LOG(LogAzSpeech, Warning, 
				TEXT("AzSpeech - %s: Note: Synthesizers currently only support language detection from open range"), 
				*FString(__func__));

			SpeechConfig->SetProperty(PropertyId::SpeechServiceConnection_SingleLanguageIdPriority, "Latency");
			return SpeechSynthesizer::FromConfig(SpeechConfig,
												 AutoDetectSourceLanguageConfig::FromOpenRange(),
												 InAudioConfig);
		}

		if (InLanguage.empty() || InVoiceName.empty())
		{
			UE_LOG(LogAzSpeech, Error, 
				TEXT("AzSpeech - %s: Task failed. Result: Invalid language or voice name"), 
				*FString(__func__));

			return nullptr;
		}

		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Using language: %s"), 
			*FString(__func__), *FString(UTF8_TO_TCHAR(InLanguage.c_str())));

		SpeechConfig->SetSpeechSynthesisLanguage(InLanguage);

		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Using voice: %s"), 
			*FString(__func__), *FString(UTF8_TO_TCHAR(InVoiceName.c_str())));

		SpeechConfig->SetSpeechSynthesisVoiceName(InVoiceName);

		return SpeechSynthesizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	static std::shared_ptr<SpeechRecognizer> GetAzureRecognizer(const std::shared_ptr<AudioConfig>& InAudioConfig = AudioConfig::FromDefaultMicrophoneInput(),
																const std::string& InLanguage = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		const auto SpeechConfig = SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));
		
		SpeechConfig->SetProfanity(ProfanityOption::Raw);

		if (FString(UTF8_TO_TCHAR(InLanguage.c_str())).Equals("Auto", ESearchCase::IgnoreCase))
		{
			SpeechConfig->SetProperty(PropertyId::SpeechServiceConnection_SingleLanguageIdPriority, "Latency");

			const std::vector<std::string> Candidates = GetCandidateLanguages();
			
			if (Candidates.empty())
			{
				UE_LOG(LogAzSpeech, Error, 
					TEXT("AzSpeech - %s: Task failed. Result: Invalid candidate languages"), 
					*FString(__func__));

				return nullptr;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing language auto detection"), *FString(__func__));
			for (const std::string& Iterator : Candidates)
			{
				UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Candidate: %s"), 
					*FString(__func__), *FString(UTF8_TO_TCHAR(Iterator.c_str())));
			}
			
			return SpeechRecognizer::FromConfig(SpeechConfig,
			                                    AutoDetectSourceLanguageConfig::FromLanguages(Candidates),
			                                    InAudioConfig);
		}

		if (InLanguage.empty())
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task failed. Result: Invalid language"), *FString(__func__));
			return nullptr;
		}
		
		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Using language: %s"),
			*FString(__func__), *FString(UTF8_TO_TCHAR(InLanguage.c_str())));

		SpeechConfig->SetSpeechRecognitionLanguage(InLanguage);
		SpeechConfig->SetSpeechSynthesisLanguage(InLanguage);

		return SpeechRecognizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	static bool ProcessAzSpeechResult(const ResultReason& Result)
	{
		switch (Result)
		{
			case ResultReason::Canceled:
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task failed. Reason: Canceled"), *FString(__func__));
				return false;

			case ResultReason::NoMatch:
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task failed. Reason: NoMatch"), *FString(__func__));
				return false;

			case ResultReason::SynthesizingAudioCompleted:
				UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Task completed. Reason: SynthesizingAudioCompleted"), *FString(__func__));
				return true;

			case ResultReason::RecognizedSpeech:
				UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Task completed. Reason: RecognizedSpeech"), *FString(__func__));
				return true;

			default:
				UE_LOG(LogAzSpeech, Warning, TEXT("AzSpeech - %s: Undefined reason"), *FString(__func__));
				return false;
		}
	}
}
