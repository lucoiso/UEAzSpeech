// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Structures/AzSpeechSettingsOptions.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechSettingsOptions)
#endif

bool IsAutoLanguage(const FName& Value)
{
	return Value.IsEqual("Auto", ENameCase::IgnoreCase);
}

bool IsDefault(const FName& Value)
{
	return Value.IsEqual("Default", ENameCase::IgnoreCase) || AzSpeech::Internal::HasEmptyParam(Value);
}

FAzSpeechSettingsOptions::FAzSpeechSettingsOptions()
{
	SetDefaults();
}

void FAzSpeechSettingsOptions::SetDefaults()
{
	SubscriptionOptions.SetDefaults();
	RecognitionOptions.SetDefaults();
	SynthesisOptions.SetDefaults();
}

FAzSpeechSubscriptionOptions::FAzSpeechSubscriptionOptions()
{
	SetDefaults();
}

void FAzSpeechSubscriptionOptions::SyncEndpointWithRegion()
{
	if (!bUsePrivateEndpoint)
	{
		PrivateEndpoint = *FString::Format(TEXT("https://{0}.api.cognitive.microsoft.com/sts/v1.0/issuetoken"), { RegionID.ToString() });
	}
}

void FAzSpeechSubscriptionOptions::SetDefaults()
{
	if (const UAzSpeechSettings* const Settings = GetDefault<UAzSpeechSettings>())
	{
		SubscriptionKey = Settings->DefaultOptions.SubscriptionOptions.SubscriptionKey;
		RegionID = Settings->DefaultOptions.SubscriptionOptions.RegionID;
		bUsePrivateEndpoint = Settings->DefaultOptions.SubscriptionOptions.bUsePrivateEndpoint;
		PrivateEndpoint = Settings->DefaultOptions.SubscriptionOptions.PrivateEndpoint;
	}

	SyncEndpointWithRegion();
}

FAzSpeechRecognitionOptions::FAzSpeechRecognitionOptions()
{
	SetDefaults();
}

FAzSpeechRecognitionOptions::FAzSpeechRecognitionOptions(const FName& LanguageID)
{
	SetDefaults();

	if (IsAutoLanguage(LanguageID))
	{
		this->bUseLanguageIdentification = true;
		this->LanguageID = LanguageID;
	}
	else if (IsDefault(LanguageID))
	{
		this->LanguageID = GetDefault<UAzSpeechSettings>()->DefaultOptions.RecognitionOptions.LanguageID;
	}

	this->LanguageID = LanguageID;
}

uint8 FAzSpeechRecognitionOptions::GetMaxAllowedCandidateLanguages() const
{
	switch (LanguageIdentificationMode)
	{
		case EAzSpeechLanguageIdentificationMode::AtStart:
			return 4u;

		case EAzSpeechLanguageIdentificationMode::Continuous:
			return 10u;

		default:
			break;
	}

	return 4u;
}

void FAzSpeechRecognitionOptions::SetDefaults()
{
	if (const UAzSpeechSettings* const Settings = GetDefault<UAzSpeechSettings>())
	{
		LanguageID = Settings->DefaultOptions.RecognitionOptions.LanguageID;
		CandidateLanguages = Settings->DefaultOptions.RecognitionOptions.CandidateLanguages;
		SpeechRecognitionOutputFormat = Settings->DefaultOptions.RecognitionOptions.SpeechRecognitionOutputFormat;
		bUseLanguageIdentification = Settings->DefaultOptions.RecognitionOptions.bUseLanguageIdentification;
		ProfanityFilter = Settings->DefaultOptions.RecognitionOptions.ProfanityFilter;
		LanguageIdentificationMode = Settings->DefaultOptions.RecognitionOptions.LanguageIdentificationMode;
		SegmentationSilenceTimeoutMs = Settings->DefaultOptions.RecognitionOptions.SegmentationSilenceTimeoutMs;
		InitialSilenceTimeoutMs = Settings->DefaultOptions.RecognitionOptions.InitialSilenceTimeoutMs;
	}
}

FAzSpeechSynthesisOptions::FAzSpeechSynthesisOptions()
{
	SetDefaults();
}

FAzSpeechSynthesisOptions::FAzSpeechSynthesisOptions(const FName& LanguageID)
{
	SetDefaults();

	if (IsAutoLanguage(LanguageID))
	{
		this->bUseLanguageIdentification = true;
		this->LanguageID = LanguageID;
	}
	else if (IsDefault(LanguageID))
	{
		this->LanguageID = GetDefault<UAzSpeechSettings>()->DefaultOptions.SynthesisOptions.LanguageID;
	}

	this->LanguageID = LanguageID;
}

FAzSpeechSynthesisOptions::FAzSpeechSynthesisOptions(const FName& LanguageID, const FName& VoiceName)
{
	SetDefaults();

	if (IsAutoLanguage(LanguageID))
	{
		this->bUseLanguageIdentification = true;
		this->LanguageID = LanguageID;
	}
	else if (IsDefault(LanguageID))
	{
		this->LanguageID = GetDefault<UAzSpeechSettings>()->DefaultOptions.SynthesisOptions.LanguageID;
	}

	this->LanguageID = LanguageID;
	this->VoiceName = IsDefault(VoiceName) ? GetDefault<UAzSpeechSettings>()->DefaultOptions.SynthesisOptions.VoiceName : VoiceName;
}

void FAzSpeechSynthesisOptions::SetDefaults()
{
	if (const UAzSpeechSettings* const Settings = GetDefault<UAzSpeechSettings>())
	{
		LanguageID = Settings->DefaultOptions.SynthesisOptions.LanguageID;
		VoiceName = Settings->DefaultOptions.SynthesisOptions.VoiceName;
		bEnableViseme = Settings->DefaultOptions.SynthesisOptions.bEnableViseme;
		SpeechSynthesisOutputFormat = Settings->DefaultOptions.SynthesisOptions.SpeechSynthesisOutputFormat;
		bUseLanguageIdentification = Settings->DefaultOptions.SynthesisOptions.bUseLanguageIdentification;
		ProfanityFilter = Settings->DefaultOptions.SynthesisOptions.ProfanityFilter;
		LanguageIdentificationMode = Settings->DefaultOptions.SynthesisOptions.LanguageIdentificationMode;
	}
}
