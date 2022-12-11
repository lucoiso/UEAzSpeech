// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>
#include <map>
#include <string>
#include "AzSpeech/AzSpeechRecognitionMap.h"
#include "AzSpeechPhraseListMap.h"
#include "AzSpeechSettings.generated.h"

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

/**
 * 
 */
UCLASS(Config = Plugins, DefaultConfig, meta = (DisplayName="AzSpeech"))
class AZSPEECH_API UAzSpeechSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	explicit UAzSpeechSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static const UAzSpeechSettings* Get();

	static constexpr unsigned MaxCandidateLanguages = 10u;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK API Access Key"))
	FString APIAccessKey;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK Region ID"))
	FString RegionID;

	/* It will be used if no value is specified or "Default" is passed as Language ID parameter */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK Default Language ID"))
	FString LanguageID;

	/* It will be used if no value is specified or "Default" is passed as Voice Name parameter */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK Default Voice Name"))
	FString VoiceName;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK Profanity Filter"))
	EAzSpeechProfanityFilter ProfanityFilter;

	/* It will be used if "Auto" is passed as Language ID parameter - Will use Azure SDK Language Identification */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK Auto Candidate Languages"))
	TArray<FString> AutoCandidateLanguages;

	/* Time limit in seconds to wait for related asynchronous tasks to complete */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Tasks Timeout in Seconds", ClampMin = "1", UIMin = "1"))
	int32 TimeOutInSeconds;

	/* If enabled, synthesizers tasks will generate Viseme data */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Enable Viseme"))
	bool bEnableViseme;

	/* CPU thread priority to use in created runnable threads */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Tasks Thread Priority"))
	EAzSpeechThreadPriority TasksThreadPriority;

	/* Thread update interval: Sleep time between task update checks */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Thread Update Interval", ClampMin = "0.0001", UIMin = "0.0001"))
	float ThreadUpdateInterval;

	/* If enabled, logs will be generated inside Saved/Logs/AzSpeech folder whenever a task fails */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Enable Azure SDK Logs"))
	bool bEnableSDKLogs;

	/* Will print extra internal informations in log */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Enable Internal Logs"))
	bool bEnableInternalLogs;

	/* Will print extra debugging informations in log */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Enable Debugging Logs"))
	bool bEnableDebuggingLogs;

	/* Map of Phrase Lists used to improve recognition accuracy */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Phrase List Map", TitleProperty = "Group: {GroupName}"))
	TArray<FAzSpeechPhraseListMap> PhraseListMap;

	/* String delimiters to use in recognition checks */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "String Delimiters"))
	FString StringDelimiters;

	/* Map of keywords to trigger or ignore in recognition interactions: Used by CheckReturnFromRecognitionMap task */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Recognition Map", TitleProperty = "Group: {GroupName}"))
	TArray<FAzSpeechRecognitionMap> RecognitionMap;

protected:
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	virtual void PostInitProperties() override;

private:
	void ValidateCandidateLanguages();
	void ToggleInternalLogs();
	void ValidateRecognitionMap();
	void ValidatePhraseList();

public:
	static const std::map<int, std::string> GetAzSpeechKeys();
	static const bool CheckAzSpeechSettings();
};
