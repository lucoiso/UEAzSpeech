// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
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
		else if constexpr (std::is_base_of<FName, Ty>())
		{
			return Arg1.IsNone();
		}
		else if constexpr (std::is_base_of<std::string, Ty>())
		{
			return Arg1.empty();
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

	const std::vector<std::string> GetCandidateLanguages(const FName TaskName, const uint32 TaskId)
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Getting candidate languages"), *TaskName.ToString(), TaskId, *FString(__func__));

		std::vector<std::string> Output;

		const UAzSpeechSettings* const Settings = GetPluginSettings();
		for (const FString& Iterator : Settings->AutoCandidateLanguages)
		{
			if (HasEmptyParam(Iterator))
			{
				continue;
			}

			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Using language %s as candidate"), *TaskName.ToString(), TaskId, *FString(__func__), *Iterator);

			Output.push_back(TCHAR_TO_UTF8(*Iterator));

			if (Output.size() >= UAzSpeechSettings::MaxCandidateLanguages)
			{
				Output.resize(UAzSpeechSettings::MaxCandidateLanguages);
				break;
			}
		}

		return Output;
	}

	const int32 GetTimeout()
	{
		if (const UAzSpeechSettings* const Settings = GetPluginSettings())
		{
			return Settings->TimeOutInSeconds <= 0.f ? 15.f : Settings->TimeOutInSeconds;
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

	const FAzSpeechRecognitionMap GetRecognitionMap(const FName& InGroup)
	{
		if (HasEmptyParam(InGroup))
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Invalid group name"), *FString(__func__));
			return FAzSpeechRecognitionMap();
		}

		if (const UAzSpeechSettings* const Settings = GetPluginSettings())
		{
			for (const FAzSpeechRecognitionMap& RecognitionData : Settings->RecognitionMap)
			{
				if (RecognitionData.GroupName.IsEqual(InGroup))
				{
					if (HasEmptyParam(RecognitionData.RecognitionData))
					{
						UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Recognition map group %s has empty data"), *FString(__func__), *RecognitionData.GroupName.ToString());
						return FAzSpeechRecognitionMap();
					}

					return RecognitionData;
				}
			}
		}

		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Group with name %s not found"), *FString(__func__), *InGroup.ToString());
		return FAzSpeechRecognitionMap();
	}

	const FString GetAzSpeechLogsBaseDir()
	{
		return FPaths::ProjectSavedDir() + "Logs/UEAzSpeech";
	}

	const EThreadPriority GetCPUThreadPriority()
	{
		if (const UAzSpeechSettings* const Settings = GetPluginSettings())
		{
			switch (Settings->TasksThreadPriority)
			{
				case EAzSpeechThreadPriority::Lowest:
					return EThreadPriority::TPri_Lowest;

				case EAzSpeechThreadPriority::BelowNormal:
					return EThreadPriority::TPri_Lowest;
				
				case EAzSpeechThreadPriority::Normal:
					return EThreadPriority::TPri_Lowest;

				case EAzSpeechThreadPriority::AboveNormal:
					return EThreadPriority::TPri_Lowest;

				case EAzSpeechThreadPriority::Highest:				
					return EThreadPriority::TPri_Lowest;
				
				default:
					break;
			}
		}

		return EThreadPriority::TPri_Normal;
	}

	const float GetThreadUpdateInterval()
	{
		if (const UAzSpeechSettings* const Settings = GetPluginSettings())
		{
			return Settings->ThreadUpdateInterval <= 0.f ? 0.1f : Settings->ThreadUpdateInterval;
		}

		return 0.1f;
	}

	const FString GetStringDelimiters()
	{
		if (const UAzSpeechSettings* const Settings = GetPluginSettings())
		{
			return Settings->StringDelimiters;
		}

		return FString(" ,.;:[]{}!'\"?");
	}
}
