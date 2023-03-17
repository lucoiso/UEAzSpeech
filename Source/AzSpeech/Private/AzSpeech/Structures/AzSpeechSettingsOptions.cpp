// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Structures/AzSpeechSettingsOptions.h"
#include "AzSpeech/AzSpeechSettings.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechSettingsOptions)
#endif

FAzSpeechSettingsOptions::FAzSpeechSettingsOptions()
{
	SetDefaults();
}

FAzSpeechSettingsOptions::FAzSpeechSettingsOptions(const FString& LanguageID)
{
	SetDefaults();

	this->LanguageID = LanguageID;
}

FAzSpeechSettingsOptions::FAzSpeechSettingsOptions(const FString& LanguageID, const FString& VoiceName)
{
	SetDefaults();

	this->LanguageID = LanguageID;
	this->VoiceName = VoiceName;
}

void FAzSpeechSettingsOptions::SetDefaults()
{
	if (const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get())
	{
		SubscriptionKey = Settings->Options.SubscriptionKey;
		RegionID = Settings->Options.RegionID;
		bUsePrivateEndpoint = Settings->Options.bUsePrivateEndpoint;
		PrivateEndpoint = Settings->Options.PrivateEndpoint;
		LanguageID = Settings->Options.LanguageID;
		AutoCandidateLanguages = Settings->Options.AutoCandidateLanguages;
		VoiceName = Settings->Options.VoiceName;
		ProfanityFilter = Settings->Options.ProfanityFilter;
		SegmentationSilenceTimeoutMs = Settings->Options.SegmentationSilenceTimeoutMs;
		InitialSilenceTimeoutMs = Settings->Options.InitialSilenceTimeoutMs;
		bEnableViseme = Settings->Options.bEnableViseme;
		bFilterVisemeFacialExpression = Settings->Options.bFilterVisemeFacialExpression;
		SpeechSynthesisOutputFormat = Settings->Options.SpeechSynthesisOutputFormat;
		SpeechRecognitionOutputFormat = Settings->Options.SpeechRecognitionOutputFormat;
		TimeOutInSeconds = Settings->Options.TimeOutInSeconds;
		TasksThreadPriority = Settings->Options.TasksThreadPriority;
		ThreadUpdateInterval = Settings->Options.ThreadUpdateInterval;
	}
}
