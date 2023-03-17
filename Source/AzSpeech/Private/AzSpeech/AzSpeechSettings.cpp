// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"

#if WITH_EDITOR
#include <Misc/MessageDialog.h>
#endif

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechSettings)
#endif

UAzSpeechSettings::UAzSpeechSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), bEnableSDKLogs(true), bEnableInternalLogs(false), bEnableDebuggingLogs(false), StringDelimiters(" ,.;:[]{}!'\"?")
{
	CategoryName = TEXT("Plugins");

	Options.SubscriptionKey = NAME_None;
	Options.RegionID = NAME_None;
	Options.bUsePrivateEndpoint = false;
	Options.PrivateEndpoint = NAME_None;
	Options.LanguageID = NAME_None;
	Options.VoiceName = NAME_None;
	Options.ProfanityFilter = EAzSpeechProfanityFilter::Raw;
	Options.SegmentationSilenceTimeoutMs = 1000;
	Options.InitialSilenceTimeoutMs = 5000;
	Options.bEnableViseme = true;
	Options.bFilterVisemeFacialExpression = true;
	Options.SpeechSynthesisOutputFormat = EAzSpeechSynthesisOutputFormat::Riff16Khz16BitMonoPcm;
	Options.SpeechRecognitionOutputFormat = EAzSpeechRecognitionOutputFormat::Detailed;
	Options.TimeOutInSeconds = 10.f;
	Options.TasksThreadPriority = EAzSpeechThreadPriority::Normal;
	Options.ThreadUpdateInterval = 0.033334f;

	if (AzSpeech::Internal::HasEmptyParam(Options.AutoCandidateLanguages))
	{
		Options.AutoCandidateLanguages.Add(Options.LanguageID);
	}
}

const UAzSpeechSettings* UAzSpeechSettings::Get()
{
	static const UAzSpeechSettings* const Instance = GetDefault<UAzSpeechSettings>();
	return Instance;
}

TArray<FName> UAzSpeechSettings::GetCandidateLanguages()
{
	return GetDefault<UAzSpeechSettings>()->Options.AutoCandidateLanguages;
}

TArray<FAzSpeechPhraseListMap> UAzSpeechSettings::GetPhraseListMap()
{
	return GetDefault<UAzSpeechSettings>()->PhraseListMap;
}

TArray<FAzSpeechRecognitionMap> UAzSpeechSettings::GetRecognitionMap()
{
	return GetDefault<UAzSpeechSettings>()->RecognitionMap;
}

FName UAzSpeechSettings::GetStringDelimiters()
{
	return GetDefault<UAzSpeechSettings>()->StringDelimiters;
}

FAzSpeechSettingsOptions UAzSpeechSettings::GetDefaultSettings()
{
	return GetDefault<UAzSpeechSettings>()->Options;
}

void UAzSpeechSettings::SetDefaultSettings(const FAzSpeechSettingsOptions& Value)
{
	GetMutableDefault<UAzSpeechSettings>()->Options = Value;
}

#if WITH_EDITOR
void UAzSpeechSettings::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(FAzSpeechSettingsOptions, LanguageID))
	{
		Options.AutoCandidateLanguages.Remove(Options.LanguageID);
	}
}

void UAzSpeechSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FAzSpeechSettingsOptions, AutoCandidateLanguages) || PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FAzSpeechSettingsOptions, LanguageID))
	{
		ValidateCandidateLanguages();
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FAzSpeechSettingsOptions, AutoCandidateLanguages))
	{
		if (Options.AutoCandidateLanguages.Num() > MaxCandidateLanguages)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("You can only include up to 4 languages for at-start LID and up to 10 languages for continuous LID."));
			Options.AutoCandidateLanguages.RemoveAtSwap(MaxCandidateLanguages, Options.AutoCandidateLanguages.Num() - MaxCandidateLanguages, true);
		}
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, bEnableInternalLogs) || PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, bEnableDebuggingLogs))
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
		Options.AutoCandidateLanguages.Remove(NAME_None);
	}

	if (!Options.AutoCandidateLanguages.Contains(Options.LanguageID))
	{
		Options.AutoCandidateLanguages.Insert(Options.LanguageID, 0);
	}

	Options.AutoCandidateLanguages.Shrink();
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
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Recognition Map has a group with invalid name."), *FString(__func__));
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
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Phrase List Map has a group with invalid name."), *FString(__func__));
		}

		for (const FString& Data : PhraseListData.Data)
		{
			if (AzSpeech::Internal::HasEmptyParam(Data))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("%s: Phrase List Map Group %s contains empty objects"), *FString(__func__));
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

	const auto UpdateSettingsMap = [&Output](const int& InId, const FName& InString)
	{
		const std::string InStr = TCHAR_TO_UTF8(*InString.ToString());
		Output.insert(std::make_pair(InId, InStr));
	};

	UpdateSettingsMap(AZSPEECH_KEY_SUBSCRIPTION, Instance->Options.SubscriptionKey);
	UpdateSettingsMap(AZSPEECH_KEY_REGION, Instance->Options.RegionID);
	UpdateSettingsMap(AZSPEECH_KEY_ENDPOINT, Instance->Options.PrivateEndpoint);
	UpdateSettingsMap(AZSPEECH_KEY_LANGUAGE, Instance->Options.LanguageID);
	UpdateSettingsMap(AZSPEECH_KEY_VOICE, Instance->Options.VoiceName);

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

	const bool bUsingEndpoint = UAzSpeechSettings::Get()->Options.bUsePrivateEndpoint;

	for (uint8 Iterator = 0u; Iterator < AzSpeechParams.size(); ++Iterator)
	{
		if (bUsingEndpoint && Iterator == AZSPEECH_KEY_REGION)
		{
			// Do not check region
			continue;
		}
		else if (!bUsingEndpoint && Iterator == AZSPEECH_KEY_ENDPOINT)
		{
			// Do not check endpoint
			continue;
		}

		if (AzSpeechParams.at(Iterator).empty())
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Invalid settings. Check your AzSpeech settings on Project Settings -> AzSpeech Settings."), *FString(__func__));
			return false;
		}
	}

	return true;
}
