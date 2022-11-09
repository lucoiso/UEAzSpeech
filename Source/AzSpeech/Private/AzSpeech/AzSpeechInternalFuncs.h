// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once
#include "CoreMinimal.h"
#include "AzSpeech.h"
#include "AzSpeech/AzSpeechSettings.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx.h>
THIRD_PARTY_INCLUDES_END

namespace AzSpeech::Internal
{
	template<typename Ty>
	const bool HasEmptyParam(const Ty& Arg1)
	{
		if constexpr (std::is_base_of<FString, Ty>())
		{
			return Arg1.IsEmpty();
		}
		else
		{
#if ENGINE_MAJOR_VERSION >= 5
			return Arg1.IsEmpty();
#else
			return Arg1.Num() == 0;
#endif
		}
	}

	template<typename Ty, typename ...Args>
	const bool HasEmptyParam(const Ty& Arg1, Args&& ...args)
	{
		const bool bOutput = HasEmptyParam(Arg1) || HasEmptyParam(std::forward<Args>(args)...);
		if (bOutput)
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Missing parameters!"), *FString(__func__));
		}

		return bOutput;
	}

	const UAzSpeechSettings* GetPluginSettings()
	{
		static const UAzSpeechSettings* const Instance = GetDefault<UAzSpeechSettings>();
		return Instance;
	}

	const std::map<int, std::string> GetAzSpeechKeys()
	{
		std::map<int, std::string> Output;
		const UAzSpeechSettings* const Settings = GetPluginSettings();

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

	const bool CheckAzSpeechSettings()
	{
		const auto AzSpeechParams = GetAzSpeechKeys();
		if (AzSpeechParams.empty())
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Invalid settings. Aborting."), *FString(__func__));
			return false;
		}

		for (uint8 Iterator = 0u; Iterator < AzSpeechParams.size(); ++Iterator)
		{
			if (AzSpeechParams.at(Iterator).empty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("%s: Invalid settings. Check your AzSpeech settings on Project Settings -> AzSpeech Settings."), *FString(__func__));
				return false;
			}
		}

		return true;
	}

	const std::vector<std::string> GetCandidateLanguages()
	{
		std::vector<std::string> Output;

		const UAzSpeechSettings* const Settings = GetPluginSettings();
		for (const FString& Iterator : Settings->AutoLanguageCandidates)
		{
			if (HasEmptyParam(Iterator))
			{
				continue;
			}

			Output.push_back(TCHAR_TO_UTF8(*Iterator));
		}

		return Output;
	}

	const int32 GetTimeout()
	{
		if (const UAzSpeechSettings* const Settings = GetPluginSettings())
		{
			return Settings->TimeOutInSeconds;
		}

		return 15.f;
	}

	const Microsoft::CognitiveServices::Speech::ProfanityOption GetProfanityFilter()
	{
		if (const UAzSpeechSettings* const Settings = GetPluginSettings())
		{
			switch (Settings->ProfanityFilter)
			{
				case EAzSpeechProfanityFilter::Raw :
					return Microsoft::CognitiveServices::Speech::ProfanityOption::Raw;

				case EAzSpeechProfanityFilter::Removed:
					return Microsoft::CognitiveServices::Speech::ProfanityOption::Removed;

				case EAzSpeechProfanityFilter::Masked:
					return Microsoft::CognitiveServices::Speech::ProfanityOption::Masked;
					
				default: break;
			}
		}

		return Microsoft::CognitiveServices::Speech::ProfanityOption::Raw;
	}

	const FString GetLanguageID(const FString& InTestId = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		if (HasEmptyParam(InTestId) || InTestId.Equals("Default", ESearchCase::IgnoreCase))
		{
			return UTF8_TO_TCHAR(Settings.at(2).c_str());
		}

		return InTestId;
	}

	const FString GetVoiceName(const FString& InTestId = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		if (HasEmptyParam(InTestId) || InTestId.Equals("Default", ESearchCase::IgnoreCase))
		{
			return UTF8_TO_TCHAR(Settings.at(3).c_str());
		}

		return InTestId;
	}

	const FString GetAzSpeechLogsBaseDir()
	{
		return FPaths::ProjectSavedDir() + "Logs/UEAzSpeech";
	}
}
