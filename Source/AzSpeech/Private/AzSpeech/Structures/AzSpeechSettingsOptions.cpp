// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Structures/AzSpeechSettingsOptions.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechSettingsOptions)
#endif

bool IsAutoLanguage(const FName& InValue)
{
    return InValue.IsEqual("Auto", ENameCase::IgnoreCase);
}

bool IsDefault(const FName& InValue)
{
    return InValue.IsEqual("Default", ENameCase::IgnoreCase) || AzSpeech::Internal::HasEmptyParam(InValue);
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

FAzSpeechRecognitionOptions::FAzSpeechRecognitionOptions(const FName& InLocale)
{
    SetDefaults();

    if (IsAutoLanguage(InLocale))
    {
        bUseLanguageIdentification = true;
        Locale = InLocale;
    }
    else if (IsDefault(InLocale))
    {
        Locale = GetDefault<UAzSpeechSettings>()->DefaultOptions.RecognitionOptions.Locale;
    }
    else
    {
        Locale = InLocale;
    }
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
        Locale = Settings->DefaultOptions.RecognitionOptions.Locale;
        CandidateLanguages = Settings->DefaultOptions.RecognitionOptions.CandidateLanguages;
        SpeechRecognitionOutputFormat = Settings->DefaultOptions.RecognitionOptions.SpeechRecognitionOutputFormat;
        bUseLanguageIdentification = Settings->DefaultOptions.RecognitionOptions.bUseLanguageIdentification;
        ProfanityFilter = Settings->DefaultOptions.RecognitionOptions.ProfanityFilter;
        LanguageIdentificationMode = Settings->DefaultOptions.RecognitionOptions.LanguageIdentificationMode;
        SegmentationSilenceTimeoutMs = Settings->DefaultOptions.RecognitionOptions.SegmentationSilenceTimeoutMs;
        InitialSilenceTimeoutMs = Settings->DefaultOptions.RecognitionOptions.InitialSilenceTimeoutMs;
        KeywordRecognitionModelPath = Settings->DefaultOptions.RecognitionOptions.KeywordRecognitionModelPath;
    }
}

FAzSpeechSynthesisOptions::FAzSpeechSynthesisOptions()
{
    SetDefaults();
}

FAzSpeechSynthesisOptions::FAzSpeechSynthesisOptions(const FName& InLocale)
{
    SetDefaults();

    if (IsAutoLanguage(InLocale))
    {
        bUseLanguageIdentification = true;
        Locale = InLocale;
    }
    else if (IsDefault(InLocale))
    {
        Locale = GetDefault<UAzSpeechSettings>()->DefaultOptions.SynthesisOptions.Locale;
    }
    else
    {
        Locale = InLocale;
    }
}

FAzSpeechSynthesisOptions::FAzSpeechSynthesisOptions(const FName& InLocale, const FName& InVoice)
{
    SetDefaults();

    if (IsAutoLanguage(InLocale))
    {
        bUseLanguageIdentification = true;
        Locale = InLocale;
    }
    else if (IsDefault(InLocale))
    {
        Locale = GetDefault<UAzSpeechSettings>()->DefaultOptions.SynthesisOptions.Locale;
    }
    else
    {
        Locale = InLocale;
    }

    Voice = IsDefault(InVoice) ? GetDefault<UAzSpeechSettings>()->DefaultOptions.SynthesisOptions.Voice : InVoice;
}

void FAzSpeechSynthesisOptions::SetDefaults()
{
    if (const UAzSpeechSettings* const Settings = GetDefault<UAzSpeechSettings>())
    {
        Locale = Settings->DefaultOptions.SynthesisOptions.Locale;
        Voice = Settings->DefaultOptions.SynthesisOptions.Voice;
        bEnableViseme = Settings->DefaultOptions.SynthesisOptions.bEnableViseme;
        SpeechSynthesisOutputFormat = Settings->DefaultOptions.SynthesisOptions.SpeechSynthesisOutputFormat;
        bUseLanguageIdentification = Settings->DefaultOptions.SynthesisOptions.bUseLanguageIdentification;
        ProfanityFilter = Settings->DefaultOptions.SynthesisOptions.ProfanityFilter;
        LanguageIdentificationMode = Settings->DefaultOptions.SynthesisOptions.LanguageIdentificationMode;
    }
}