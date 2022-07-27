// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once
#include "CoreMinimal.h"
#include "AzSpeechSettings.h"

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
		UpdateSettingsMap(2, Settings->LanguageId);
		UpdateSettingsMap(3, Settings->VoiceName);

		return Output;
	}
	
	static FString GetLanguageId(const FString InTestId = "")
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

	static std::shared_ptr<SpeechSynthesizer> GetAzureSynthesizer(const std::shared_ptr<AudioConfig>& InAudioConfig =
		                                                              AudioConfig::FromDefaultSpeakerOutput(),
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

		return SpeechSynthesizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	static std::shared_ptr<SpeechRecognizer> GetAzureRecognizer(const std::shared_ptr<AudioConfig>& InAudioConfig = nullptr,
	                                                            const std::string& InLanguage = "")
	{
		const auto& Settings = GetAzSpeechKeys();
		const auto& SpeechConfig = SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));

		if (!InLanguage.empty())
		{
			SpeechConfig->SetSpeechRecognitionLanguage(InLanguage);
			SpeechConfig->SetSpeechSynthesisLanguage(InLanguage);
		}
		SpeechConfig->SetProfanity(ProfanityOption::Raw);

		return SpeechRecognizer::FromConfig(SpeechConfig, InAudioConfig);
	}
}
