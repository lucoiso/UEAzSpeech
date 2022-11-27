// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "LogAzSpeech.h"
#include "AzSpeech/AzSpeechSettings.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_enums.h>
THIRD_PARTY_INCLUDES_END

#include <map>
#include <vector>
#include <string>

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
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Missing parameters!"), *FString(__func__));
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
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Invalid settings. Check your AzSpeech settings on Project Settings -> AzSpeech Settings."), *FString(__func__));
			return false;
		}

		for (uint8 Iterator = 0u; Iterator < AzSpeechParams.size(); ++Iterator)
		{
			if (AzSpeechParams.at(Iterator).empty())
			{
				UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Invalid settings. Check your AzSpeech settings on Project Settings -> AzSpeech Settings."), *FString(__func__));
				return false;
			}
		}

		return true;
	}

	const std::vector<std::string> GetCandidateLanguages(const FName TaskName, const uint32 TaskId, const bool bContinuous)
	{
		const unsigned Quantity = bContinuous ? UAzSpeechSettings::MaxContinuousCandidateLanguages : UAzSpeechSettings::MaxAtStartCandidateLanguages;
		const FString AutoDetectModeStr = bContinuous ? "Continuous" : "At-Start";

		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%s); Function: %s; Message: Getting candidate languages. Mode: %s; Candidates: %d"), *TaskName.ToString(), *FString::FromInt(TaskId), *FString(__func__), *AutoDetectModeStr, Quantity);

		std::vector<std::string> Output;

		const UAzSpeechSettings* const Settings = GetPluginSettings();
		for (const FString& Iterator : Settings->AutoCandidateLanguages)
		{
			if (HasEmptyParam(Iterator))
			{
				continue;
			}

			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%s); Function: %s; Message: Using language %s as candidate"), *TaskName.ToString(), *FString::FromInt(TaskId), *FString(__func__), *Iterator);

			Output.push_back(TCHAR_TO_UTF8(*Iterator));

			if (Output.size() >= 3)
			{
				break;
			}
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

	void GetLanguageID(FString& InLanguage)
	{
		const auto Settings = GetAzSpeechKeys();
		if (HasEmptyParam(InLanguage) || InLanguage.Equals("Default", ESearchCase::IgnoreCase))
		{
			InLanguage = UTF8_TO_TCHAR(Settings.at(2).c_str());
		}
	}

	void GetVoiceName(FString& InVoice)
	{
		const auto Settings = GetAzSpeechKeys();
		if (HasEmptyParam(InVoice) || InVoice.Equals("Default", ESearchCase::IgnoreCase))
		{
			InVoice = UTF8_TO_TCHAR(Settings.at(3).c_str());
		}
	}

	const FString GetAzSpeechLogsBaseDir()
	{
		return FPaths::ProjectSavedDir() + "Logs/UEAzSpeech";
	}

	const ENamedThreads::Type GetBackgroundThread()
	{
		if (const UAzSpeechSettings* const Settings = GetPluginSettings())
		{
			if (Settings->bUseHighPriorityThreads)
			{
				return ENamedThreads::AnyBackgroundHiPriTask;
			}
		}

		return ENamedThreads::AnyBackgroundThreadNormalTask;
	}
}
