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

		const auto& UpdateSettingsMap = [&Output](const int& InId, const FString& InString)
		{
			const std::string& InStr = TCHAR_TO_UTF8(*InString);
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
		for (const auto& Iterator : Settings->AutoLanguageCandidates)
		{
			Output.push_back(TCHAR_TO_UTF8(*Iterator));
		}

		return Output;
	}

	static FString GetLanguageID(const FString InTestId = "")
	{
		const auto& Settings = GetAzSpeechKeys();
		if (InTestId.IsEmpty() || InTestId == "Default")
		{
			return UTF8_TO_TCHAR(Settings.at(2).c_str());
		}

		return InTestId;
	}

	static FString GetVoiceName(const FString InTestId = "")
	{
		const auto& Settings = GetAzSpeechKeys();
		if (InTestId.IsEmpty() || InTestId == "Default")
		{
			return UTF8_TO_TCHAR(Settings.at(3).c_str());
		}

		return InTestId;
	}

	static std::shared_ptr<SpeechSynthesizer> GetAzureSynthesizer(
		const std::shared_ptr<AudioConfig>& InAudioConfig = AudioConfig::FromDefaultSpeakerOutput(),
		const std::string& InLanguage = "",
		const std::string& InVoiceName = "")
	{
		const auto& Settings = GetAzSpeechKeys();
		const auto& SpeechConfig = SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));

		if (!InLanguage.empty())
		{
			SpeechConfig->SetSpeechSynthesisLanguage(InLanguage);
		}
		if (!InVoiceName.empty())
		{
			SpeechConfig->SetSpeechSynthesisVoiceName(InVoiceName);
		}

		if (InLanguage == "auto" || InLanguage == "Auto")
		{
			SpeechConfig->SetProperty(PropertyId::SpeechServiceConnection_SingleLanguageIdPriority, "Latency");
			return SpeechSynthesizer::FromConfig(SpeechConfig,
			                                     AutoDetectSourceLanguageConfig::FromOpenRange(),
			                                     InAudioConfig);
		}

		return SpeechSynthesizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	static std::shared_ptr<SpeechRecognizer> GetAzureRecognizer(
		const std::shared_ptr<AudioConfig>& InAudioConfig = AudioConfig::FromDefaultMicrophoneInput(),
		const std::string& InLanguage = "")
	{
		const auto& Settings = GetAzSpeechKeys();
		const auto& SpeechConfig = SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));
		SpeechConfig->SetProfanity(ProfanityOption::Raw);

		if (InLanguage == "auto" || InLanguage == "Auto")
		{
			SpeechConfig->SetProperty(PropertyId::SpeechServiceConnection_SingleLanguageIdPriority, "Latency");

			const auto& Candidates = GetCandidateLanguages();
			return SpeechRecognizer::FromConfig(SpeechConfig,
			                                    AutoDetectSourceLanguageConfig::FromLanguages(Candidates),
			                                    InAudioConfig);
		}

		if (!InLanguage.empty())
		{
			SpeechConfig->SetSpeechRecognitionLanguage(InLanguage);
			SpeechConfig->SetSpeechSynthesisLanguage(InLanguage);
		}

		return SpeechRecognizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	static bool ProcessAzSpeechResult(const ResultReason Result)
	{
		switch (Result)
		{
		case ResultReason::Canceled:
			UE_LOG(LogAzSpeech, Error,
			       TEXT("AzSpeech - %s: Task failed: Canceled"),
			       *FString(__func__));
			return false;

		case ResultReason::NoMatch:
			UE_LOG(LogAzSpeech, Error,
			       TEXT("AzSpeech - %s: Task failed: NoMatch"),
			       *FString(__func__));
			return false;

		case ResultReason::SynthesizingAudioCompleted:
			UE_LOG(LogAzSpeech, Display,
			       TEXT("AzSpeech - %s: Task completed: SynthesizingAudioCompleted"),
			       *FString(__func__));
			return true;

		case ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech, Display,
			       TEXT("AzSpeech - %s: Task completed: RecognizedSpeech"),
			       *FString(__func__));
			return true;

		default:
			UE_LOG(LogAzSpeech, Warning,
			       TEXT("AzSpeech - %s: Undefined result"),
			       *FString(__func__));
			return false;
		}
	}
}
