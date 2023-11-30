// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"
#include <Runtime/Launch/Resources/Version.h>

#if WITH_EDITOR
#include <Misc/MessageDialog.h>
#endif

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechSettings)
#endif

UAzSpeechSettings::UAzSpeechSettings(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , TaskInitTimeOut(15.f)
    , TasksThreadPriority(EAzSpeechThreadPriority::Normal)
    , ThreadUpdateInterval(0.016667f)
    , bFilterVisemeFacialExpression(true)
    , bEnableSDKLogs(true)
    , bEnableInternalLogs(false)
    , bEnableDebuggingLogs(false)
    , bEnableDebuggingPrints(false)
    , StringDelimiters(TEXT(R"( ,.;:[]{}!'"?)"))
{
    CategoryName = TEXT("Plugins");

    SetToDefaults();
}

const UAzSpeechSettings* UAzSpeechSettings::Get()
{
    static const UAzSpeechSettings* const Instance = GetDefault<UAzSpeechSettings>();
    return Instance;
}

TArray<FName> UAzSpeechSettings::GetCandidateLanguages()
{
    return GetDefault<UAzSpeechSettings>()->DefaultOptions.RecognitionOptions.CandidateLanguages;
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

FAzSpeechSettingsOptions UAzSpeechSettings::GetDefaultOptions()
{
    return GetDefault<UAzSpeechSettings>()->DefaultOptions;
}

void UAzSpeechSettings::SetDefaultOptions(const FAzSpeechSettingsOptions& Value)
{
    UAzSpeechSettings* const Settings = GetMutableDefault<UAzSpeechSettings>();
    Settings->DefaultOptions = Value;

    Settings->SaveAndReload(GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, DefaultOptions));
}

#if WITH_EDITOR
void UAzSpeechSettings::PreEditChange(FProperty* const PropertyAboutToChange)
{
    Super::PreEditChange(PropertyAboutToChange);

    if (PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(FAzSpeechRecognitionOptions, Locale))
    {
        DefaultOptions.RecognitionOptions.CandidateLanguages.Remove(DefaultOptions.RecognitionOptions.Locale);
    }
}

void UAzSpeechSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FAzSpeechRecognitionOptions, CandidateLanguages) || PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FAzSpeechRecognitionOptions, Locale))
    {
        constexpr uint8 MaxCandidateLanguages = 10;

        if (DefaultOptions.RecognitionOptions.CandidateLanguages.Num() > MaxCandidateLanguages)
        {
            FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("You can only include up to 4 languages for at-start LID and up to 10 languages for continuous LID.")));
            DefaultOptions.RecognitionOptions.CandidateLanguages.RemoveAtSwap(MaxCandidateLanguages, DefaultOptions.RecognitionOptions.CandidateLanguages.Num() - MaxCandidateLanguages, true);
        }

        ValidateCandidateLanguages();
    }

    if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FAzSpeechSubscriptionOptions, RegionID) || PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FAzSpeechSubscriptionOptions, bUsePrivateEndpoint))
    {
        ValidateEndpoint();
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
    ValidateRecognitionMap();
    ValidateEndpoint();

    ToggleInternalLogs();
}

void UAzSpeechSettings::SetToDefaults()
{
    DefaultOptions.SubscriptionOptions.SubscriptionKey = NAME_None;
    DefaultOptions.SubscriptionOptions.RegionID = NAME_None;
    DefaultOptions.SubscriptionOptions.bUsePrivateEndpoint = false;
    DefaultOptions.SubscriptionOptions.PrivateEndpoint = NAME_None;

    DefaultOptions.SynthesisOptions.Locale = NAME_None;
    DefaultOptions.SynthesisOptions.Voice = NAME_None;
    DefaultOptions.SynthesisOptions.bEnableViseme = true;
    DefaultOptions.SynthesisOptions.SpeechSynthesisOutputFormat = EAzSpeechSynthesisOutputFormat::Riff16Khz16BitMonoPcm;
    DefaultOptions.SynthesisOptions.bUseLanguageIdentification = false;
    DefaultOptions.SynthesisOptions.ProfanityFilter = EAzSpeechProfanityFilter::Raw;
    DefaultOptions.SynthesisOptions.LanguageIdentificationMode = EAzSpeechLanguageIdentificationMode::AtStart;

    DefaultOptions.RecognitionOptions.Locale = NAME_None;
    DefaultOptions.RecognitionOptions.SpeechRecognitionOutputFormat = EAzSpeechRecognitionOutputFormat::Detailed;
    DefaultOptions.RecognitionOptions.bUseLanguageIdentification = false;
    DefaultOptions.RecognitionOptions.ProfanityFilter = EAzSpeechProfanityFilter::Raw;
    DefaultOptions.RecognitionOptions.LanguageIdentificationMode = EAzSpeechLanguageIdentificationMode::AtStart;
    DefaultOptions.RecognitionOptions.SegmentationSilenceTimeoutMs = 1000;
    DefaultOptions.RecognitionOptions.InitialSilenceTimeoutMs = 5000;

    if (AzSpeech::Internal::HasEmptyParam(DefaultOptions.RecognitionOptions.CandidateLanguages))
    {
        DefaultOptions.RecognitionOptions.CandidateLanguages.Add(DefaultOptions.RecognitionOptions.Locale);
    }
}

void UAzSpeechSettings::SaveAndReload(const FName& PropertyName)
{
    SaveConfig();

    uint32 PropagationFlags = 0u;

#if ENGINE_MAJOR_VERSION >= 5
    PropagationFlags = UE::ELoadConfigPropagationFlags::LCPF_PropagateToChildDefaultObjects;
#else
    PropagationFlags = UE4::ELoadConfigPropagationFlags::LCPF_PropagateToChildDefaultObjects;
#endif

    ReloadConfig(GetClass(), *GetDefaultConfigFilename(), PropagationFlags, GetClass()->FindPropertyByName(PropertyName));
}

void UAzSpeechSettings::ValidateCandidateLanguages(const bool bRemoveEmpties)
{
    if (bRemoveEmpties)
    {
        DefaultOptions.RecognitionOptions.CandidateLanguages.Remove(NAME_None);
    }

    if (!DefaultOptions.RecognitionOptions.CandidateLanguages.Contains(DefaultOptions.RecognitionOptions.Locale))
    {
        DefaultOptions.RecognitionOptions.CandidateLanguages.Insert(DefaultOptions.RecognitionOptions.Locale, 0);
    }

    DefaultOptions.RecognitionOptions.CandidateLanguages.Shrink();
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

void UAzSpeechSettings::ValidateEndpoint()
{
    DefaultOptions.SubscriptionOptions.SyncEndpointWithRegion();
}

const bool UAzSpeechSettings::CheckAzSpeechSettings()
{
    return CheckAzSpeechSettings(UAzSpeechSettings::Get()->DefaultOptions.SubscriptionOptions);
}

const bool UAzSpeechSettings::CheckAzSpeechSettings(const FAzSpeechSubscriptionOptions& Options)
{
    if (Options.bUsePrivateEndpoint && AzSpeech::Internal::HasEmptyParam(Options.PrivateEndpoint))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Invalid Endpoint."), *FString(__func__));
        return false;
    }

    if (!Options.bUsePrivateEndpoint && AzSpeech::Internal::HasEmptyParam(Options.RegionID))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Invalid Region ID."), *FString(__func__));
        return false;
    }

    if (AzSpeech::Internal::HasEmptyParam(Options.SubscriptionKey))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Invalid Subscription Key."), *FString(__func__));
        return false;
    }

    return true;
}