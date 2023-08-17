// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeechSettingsOptions.generated.h"

UENUM(BlueprintType, Category = "AzSpeech")
enum class EAzSpeechProfanityFilter : uint8
{
    Raw,
    Masked,
    Removed
};

UENUM(BlueprintType, Category = "AzSpeech")
enum class EAzSpeechThreadPriority : uint8
{
    Lowest,
    BelowNormal,
    Normal,
    AboveNormal,
    Highest,
};

UENUM(BlueprintType, Category = "AzSpeech")
enum class EAzSpeechSynthesisOutputFormat : uint8
{
    Riff16Khz16BitMonoPcm,
    Riff24Khz16BitMonoPcm,
    Riff48Khz16BitMonoPcm,
    Riff22050Hz16BitMonoPcm,
    Riff44100Hz16BitMonoPcm
};

UENUM(BlueprintType, Category = "AzSpeech")
enum class EAzSpeechRecognitionOutputFormat : uint8
{
    Simple,
    Detailed
};

UENUM(BlueprintType, Category = "AzSpeech")
enum class EAzSpeechLanguageIdentificationMode : uint8
{
    AtStart,
    Continuous
};

USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechSubscriptionOptions
{
    GENERATED_BODY()

    friend struct FAzSpeechSettingsOptions;

public:
    FAzSpeechSubscriptionOptions();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azure", Meta = (DisplayName = "Subscription Key"))
    FName SubscriptionKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azure", Meta = (DisplayName = "Region ID", EditCondition = "!bUsePrivateEndpoint"))
    FName RegionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azure", Meta = (DisplayName = "Use Private Endpoint"))
    bool bUsePrivateEndpoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azure", Meta = (DisplayName = "Private Endpoint", EditCondition = "bUsePrivateEndpoint"))
    FName PrivateEndpoint;

    /* If not using private endpoint, set endpoint value to: https://REGION-ID.api.cognitive.microsoft.com/sts/v1.0/issuetoken */
    void SyncEndpointWithRegion();

private:
    void SetDefaults();
};

USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechRecognitionOptions
{
    GENERATED_BODY()

    friend struct FAzSpeechSettingsOptions;

public:
    FAzSpeechRecognitionOptions();
    FAzSpeechRecognitionOptions(const FName& InLocale);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Locale"))
    FName Locale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Use Language Identification"))
    bool bUseLanguageIdentification;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Language Identification Mode"))
    EAzSpeechLanguageIdentificationMode LanguageIdentificationMode;

    /* Will use Azure SDK Language Identification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Candidate Languages"))
    TArray<FName> CandidateLanguages;

    /* Recognition output format */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Recognition Output Format"))
    EAzSpeechRecognitionOutputFormat SpeechRecognitionOutputFormat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Profanity Filter"))
    EAzSpeechProfanityFilter ProfanityFilter;

    /* Silence time limit in miliseconds to consider the task as Completed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Segmentation Silence Timeout in Miliseconds", ClampMin = "100", UIMin = "100", ClampMax = "5000", UIMax = "5000"))
    int32 SegmentationSilenceTimeoutMs;

    /* Silence time limit in miliseconds at the start of the task to consider the result as Canceled/NoMatch */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Initial Silence Timeout in Miliseconds", ClampMin = "0", UIMin = "0"))
    int32 InitialSilenceTimeoutMs;

    /* Path to the keyword recognition model to use in Keyword Recognition tasks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Keyword Recognition Model Path"))
    FString KeywordRecognitionModelPath;

    uint8 GetMaxAllowedCandidateLanguages() const;

private:
    void SetDefaults();
};

USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechSynthesisOptions
{
    GENERATED_BODY()

    friend struct FAzSpeechSettingsOptions;

public:
    FAzSpeechSynthesisOptions();
    FAzSpeechSynthesisOptions(const FName& InLocale);
    FAzSpeechSynthesisOptions(const FName& InLocale, const FName& InVoice);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Locale"))
    FName Locale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Voice"))
    FName Voice;

    /* If enabled, tasks will generate Viseme data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Enable Viseme"))
    bool bEnableViseme;

    /* Synthesis audio output format */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Synthesis Output Format"))
    EAzSpeechSynthesisOutputFormat SpeechSynthesisOutputFormat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Use Language Identification"))
    bool bUseLanguageIdentification;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Language Identification Mode"))
    EAzSpeechLanguageIdentificationMode LanguageIdentificationMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Profanity Filter"))
    EAzSpeechProfanityFilter ProfanityFilter;

private:
    void SetDefaults();
};

USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechSettingsOptions
{
    GENERATED_BODY()

public:
    FAzSpeechSettingsOptions();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azure", Meta = (DisplayName = "Subscription Options"))
    FAzSpeechSubscriptionOptions SubscriptionOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azure", Meta = (DisplayName = "Recognition Options"))
    FAzSpeechRecognitionOptions RecognitionOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azure", Meta = (DisplayName = "Synthesis Options"))
    FAzSpeechSynthesisOptions SynthesisOptions;

private:
    void SetDefaults();
};