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

FAzSpeechSettingsOptions::FAzSpeechSettingsOptions(const FName& LanguageID)
{
	SetDefaults();

	this->LanguageID = LanguageID;
}

FAzSpeechSettingsOptions::FAzSpeechSettingsOptions(const FName& LanguageID, const FName& VoiceName)
{
	SetDefaults();

	this->LanguageID = LanguageID;
	this->VoiceName = VoiceName;
}

void FAzSpeechSettingsOptions::SetDefaults()
{
	if (const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get())
	{
		SubscriptionKey = Settings->DefaultOptions.SubscriptionKey;
		RegionID = Settings->DefaultOptions.RegionID;
		bUsePrivateEndpoint = Settings->DefaultOptions.bUsePrivateEndpoint;
		PrivateEndpoint = Settings->DefaultOptions.PrivateEndpoint;
		LanguageID = Settings->DefaultOptions.LanguageID;
		AutoCandidateLanguages = Settings->DefaultOptions.AutoCandidateLanguages;
		VoiceName = Settings->DefaultOptions.VoiceName;
		ProfanityFilter = Settings->DefaultOptions.ProfanityFilter;
		SegmentationSilenceTimeoutMs = Settings->DefaultOptions.SegmentationSilenceTimeoutMs;
		InitialSilenceTimeoutMs = Settings->DefaultOptions.InitialSilenceTimeoutMs;
		bEnableViseme = Settings->DefaultOptions.bEnableViseme;
		bFilterVisemeFacialExpression = Settings->DefaultOptions.bFilterVisemeFacialExpression;
		SpeechSynthesisOutputFormat = Settings->DefaultOptions.SpeechSynthesisOutputFormat;
		SpeechRecognitionOutputFormat = Settings->DefaultOptions.SpeechRecognitionOutputFormat;
		TimeOutInSeconds = Settings->DefaultOptions.TimeOutInSeconds;
		TasksThreadPriority = Settings->DefaultOptions.TasksThreadPriority;
		ThreadUpdateInterval = Settings->DefaultOptions.ThreadUpdateInterval;
	}
}