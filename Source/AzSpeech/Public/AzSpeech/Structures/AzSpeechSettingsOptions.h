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

	/* Silence time limit in miliseconds to consider the task as Completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Segmentation Silence Timeout in Miliseconds", ClampMin = "100", UIMin = "100", ClampMax = "5000", UIMax = "5000"))
	int32 SegmentationSilenceTimeoutMs;

	/* Silence time limit in miliseconds at the start of the task to consider the result as Canceled/NoMatch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Initial Silence Timeout in Miliseconds", ClampMin = "0", UIMin = "0"))
	int32 InitialSilenceTimeoutMs;

	/* If enabled, synthesizers tasks will generate Viseme data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Enable Viseme"))
	bool bEnableViseme;

	/* If enabled, SSML synthesizers tasks with viseme output type set to FacialExpression will return only data that contains the Animation property */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Filter Viseme Facial Expression"))
	bool bFilterVisemeFacialExpression;

	/* Synthesis audio output format */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Synthesis Output Format"))
	EAzSpeechSynthesisOutputFormat SpeechSynthesisOutputFormat;

	/* Recognition output format */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks", Meta = (DisplayName = "Recognition Output Format"))
	EAzSpeechRecognitionOutputFormat SpeechRecognitionOutputFormat;

	/* Time limit in seconds to wait for related asynchronous tasks to complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thread", Meta = (DisplayName = "Attempt Timeout in Seconds", ClampMin = "1", UIMin = "1", ClampMax = "600", UIMax = "600"))
	int32 TimeOutInSeconds;

	/* CPU thread priority to use in created runnable threads */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thread", Meta = (DisplayName = "Thread Priority"))
	EAzSpeechThreadPriority TasksThreadPriority;

	/* Thread update interval: Sleep time between task update checks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thread", Meta = (DisplayName = "Thread Update Interval", ClampMin = "0.0001", UIMin = "0.0001", ClampMax = "1", UIMax = "1"))
	float ThreadUpdateInterval;

private:
	void SetDefaults();
};