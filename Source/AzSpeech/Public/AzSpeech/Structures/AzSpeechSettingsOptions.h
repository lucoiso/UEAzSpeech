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

USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechSettingsOptions
{
	GENERATED_BODY()
		
public:
	FAzSpeechSettingsOptions();
	FAzSpeechSettingsOptions(const FName& LanguageID);
	FAzSpeechSettingsOptions(const FName& LanguageID, const FName& VoiceName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azure", Meta = (DisplayName = "Subscription Key"))
	FName SubscriptionKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azure", Meta = (DisplayName = "Region ID", EditCondition = "!bUsePrivateEndpoint"))
	FName RegionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azure", Meta = (DisplayName = "Use Private Endpoint"))
	bool bUsePrivateEndpoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azure", Meta = (DisplayName = "Private Endpoint", EditCondition = "bUsePrivateEndpoint"))
	FName PrivateEndpoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Default Language ID"))
	FName LanguageID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Default Voice Name"))
	FName VoiceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Profanity Filter"))
	EAzSpeechProfanityFilter ProfanityFilter;

	/* It will be used if "Auto" is passed as Language ID parameter - Will use Azure SDK Language Identification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Auto Candidate Languages"))
	TArray<FName> AutoCandidateLanguages;

	/* If enabled, synthesizers tasks will generate Viseme data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Enable Viseme"))
	bool bEnableViseme;

	/* Synthesis audio output format */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Synthesis Output Format"))
	EAzSpeechSynthesisOutputFormat SpeechSynthesisOutputFormat;

	/* Recognition output format */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Recognition Output Format"))
	EAzSpeechRecognitionOutputFormat SpeechRecognitionOutputFormat;

private:
	void SetDefaults();
};