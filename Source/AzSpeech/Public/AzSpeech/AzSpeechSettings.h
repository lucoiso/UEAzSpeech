// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AzSpeechSettings.generated.h"

UENUM(BlueprintType, Category = "AzSpeech")
enum class EAzSpeechProfanityFilter : uint8
{
	Raw,
	Masked,
	Removed
};

/**
 * 
 */
UCLASS(Config = Plugins, DefaultConfig, meta = (DisplayName="AzSpeech"))
class AZSPEECH_API UAzSpeechSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	explicit UAzSpeechSettings(const FObjectInitializer& ObjectInitializer);

	static constexpr unsigned MaxAtStartCandidateLanguages = 4u;
	static constexpr unsigned MaxContinuousCandidateLanguages = 10u;

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
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Tasks Timeout in Seconds"))
	int32 TimeOutInSeconds;

	/* If enabled, synthesizers tasks will generate Viseme data */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Enable Viseme"))
	bool bEnableViseme;

	/* If enabled, logs will be generated inside Saved/Logs/AzSpeech folder whenever a task fails */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Enable Azure SDK Logs"))
	bool bEnableSDKLogs;

	/* Will print extra internal informations on log */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Enable Internal Logs"))
	bool bEnableInternalLogs;

	/* Will print extra debugging informations on log */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Enable Debugging Logs"))
	bool bEnableDebuggingLogs;

protected:
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	virtual void PostInitProperties() override;

private:
	void ValidateCandidateLanguages();
	void ToggleInternalLogs();
};
