// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"

#if WITH_EDITOR
#include <Misc/MessageDialog.h>
#endif // WITH_EDITOR

UAzSpeechSettings::UAzSpeechSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), TimeOutInSeconds(10.f), bEnableViseme(true), TasksThreadPriority(EAzSpeechThreadPriority::Normal), ThreadUpdateInterval(0.033334f), bEnableSDKLogs(true), bEnableInternalLogs(false), bEnableDebuggingLogs(false), StringDelimiters(" ,.;:[]{}!'\"?")
{
	CategoryName = TEXT("Plugins");

	if (AzSpeech::Internal::HasEmptyParam(AutoCandidateLanguages))
	{
		AutoCandidateLanguages.Add(LanguageID);
	}
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
#endif // WITH_EDITOR

void UAzSpeechSettings::PostInitProperties()
{
	Super::PostInitProperties();

	ValidateCandidateLanguages();
	ToggleInternalLogs();
	ValidateRecognitionMap();
}

void UAzSpeechSettings::ValidateCandidateLanguages()
{
	AutoCandidateLanguages.Remove(FString());

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
			UE_LOG(LogAzSpeech, Error, TEXT("%s: RecognitionMap has a group with invalid name."));
			continue;
		}

		for (const FAzSpeechRecognitionData& RecognitionData : RecognitionMapGroup.RecognitionData)
		{
			if (RecognitionData.Value < 0)
			{
				UE_LOG(LogAzSpeech, Error, TEXT("%s: Recognition Map Group '%s' has a Recognition Data with invalid value."), *FString(__func__), *RecognitionMapGroup.GroupName.ToString());
				break;
			}

			if (AzSpeech::Internal::HasEmptyParam(RecognitionData.TriggerKeys))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("%s: Recognition Map Group '%s' has a Recognition Data without Trigger Keys."), *FString(__func__), *RecognitionMapGroup.GroupName.ToString());
				break;
			}

			for (const FString& TriggerKey : RecognitionData.TriggerKeys)
			{
				if (AzSpeech::Internal::HasEmptyParam(TriggerKey))
				{
					UE_LOG(LogAzSpeech, Error, TEXT("%s: Recognition Map Group '%s' has a empty Trigger Key."), *FString(__func__), *RecognitionMapGroup.GroupName.ToString());
					break;
				}
			}
		}
	}
}
