// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"

#if WITH_EDITOR
#include <Misc/MessageDialog.h>
#endif

UAzSpeechSettings::UAzSpeechSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), TimeOutInSeconds(10.f), bEnableViseme(true), TasksThreadPriority(EAzSpeechThreadPriority::Normal), ThreadUpdateInterval(0.033334f), bEnableSDKLogs(true), bEnableInternalLogs(false), bEnableDebuggingLogs(false), StringDelimiters(" ,.;:[]{}!'\"?")
{
	CategoryName = TEXT("Plugins");

	if (AzSpeech::Internal::HasEmptyParam(AutoCandidateLanguages))
	{
		AutoCandidateLanguages.Add(LanguageID);
	}
}

const UAzSpeechSettings* UAzSpeechSettings::Get()
{
	static const UAzSpeechSettings* const Instance = GetDefault<UAzSpeechSettings>();
	return Instance;
}

#if WITH_EDITOR
void UAzSpeechSettings::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, LanguageID))
	{
		AutoCandidateLanguages.Remove(LanguageID);
	}
}

void UAzSpeechSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, AutoCandidateLanguages)
		|| PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, LanguageID))
	{
		ValidateCandidateLanguages();
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, AutoCandidateLanguages))
	{
		if (AutoCandidateLanguages.Num() > MaxCandidateLanguages)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("You can only include up to 4 languages for at-start LID and up to 10 languages for continuous LID."));
			AutoCandidateLanguages.RemoveAtSwap(MaxCandidateLanguages, AutoCandidateLanguages.Num() - MaxCandidateLanguages, true);
		}
	}
	
	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, bEnableInternalLogs)
		|| PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, bEnableDebuggingLogs))
	{
		ToggleInternalLogs();
	}
}
#endif

void UAzSpeechSettings::PostInitProperties()
{
	Super::PostInitProperties();

	ValidateCandidateLanguages(true);
	ToggleInternalLogs();
	ValidateRecognitionMap();
}

void UAzSpeechSettings::ValidateCandidateLanguages(const bool bRemoveEmpties)
{
	if (bRemoveEmpties)
	{
		AutoCandidateLanguages.Remove(FString());
	}

	if (!AutoCandidateLanguages.Contains(LanguageID))
	{
		AutoCandidateLanguages.Insert(LanguageID, 0);
	}

	AutoCandidateLanguages.Shrink();
}

void UAzSpeechSettings::ToggleInternalLogs()
{
#if !UE_BUILD_SHIPPING
	LogAzSpeech_Internal.SetVerbosity(bEnableInternalLogs ? ELogVerbosity::Display : ELogVerbosity::NoLogging);
	LogAzSpeech_Debugging.SetVerbosity(bEnableDebuggingLogs ? ELogVerbosity::Display : ELogVerbosity::NoLogging);
#endif
}

void UAzSpeechSettings::ValidateRecognitionMap()
{
	for (const FAzSpeechRecognitionMap& RecognitionMapGroup : RecognitionMap)
	{
		if (AzSpeech::Internal::HasEmptyParam(RecognitionMapGroup.GroupName))
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Recognition Map has a group with invalid name."));
		}

		for (const FAzSpeechRecognitionData& Data : RecognitionMapGroup.Data)
		{
			if (Data.Value < 0)
			{
				UE_LOG(LogAzSpeech, Error, TEXT("%s: Recognition Map Group %s has a Recognition Data with invalid value."), *FString(__func__), *RecognitionMapGroup.GroupName.ToString());
				break;
			}

			if (AzSpeech::Internal::HasEmptyParam(Data.TriggerKeys))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("%s: Recognition Map Group %s has a Recognition Data without Trigger Keys."), *FString(__func__), *RecognitionMapGroup.GroupName.ToString());
				break;
			}

			for (const FString& TriggerKey : Data.TriggerKeys)
			{
				if (AzSpeech::Internal::HasEmptyParam(TriggerKey))
				{
					UE_LOG(LogAzSpeech, Error, TEXT("%s: Recognition Map Group %s has a empty Trigger Key."), *FString(__func__), *RecognitionMapGroup.GroupName.ToString());
					break;
				}
			}
		}
	}
}

void UAzSpeechSettings::ValidatePhraseList()
{
	for (const FAzSpeechPhraseListMap& PhraseListData : PhraseListMap)
	{
		if (AzSpeech::Internal::HasEmptyParam(PhraseListData.GroupName))
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Phrase List Map has a group with invalid name."));
		}

		for (const FString& Data : PhraseListData.Data)
		{
			if (AzSpeech::Internal::HasEmptyParam(Data))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("%s: Phrase List Map Group %s contains empty objects"));
				break;
			}
		}
	}
}

const std::map<int, std::string> UAzSpeechSettings::GetAzSpeechKeys()
{
	std::map<int, std::string> Output;

	const UAzSpeechSettings* const Instance = UAzSpeechSettings::Get();
	if (!IsValid(Instance))
	{
		return Output;
	}

	const auto UpdateSettingsMap = [&Output](const int& InId, const FString& InString)
	{
		const std::string InStr = TCHAR_TO_UTF8(*InString);
		Output.insert(std::make_pair(InId, InStr));
	};

	UpdateSettingsMap(0, Instance->APIAccessKey);
	UpdateSettingsMap(1, Instance->RegionID);
	UpdateSettingsMap(2, Instance->LanguageID);
	UpdateSettingsMap(3, Instance->VoiceName);

	return Output;
}

const bool UAzSpeechSettings::CheckAzSpeechSettings()
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
