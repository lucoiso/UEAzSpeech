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
UCLASS(Config=Game, DefaultConfig, meta = (DisplayName="AzSpeech"))
class AZSPEECH_API UAzSpeechSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	explicit UAzSpeechSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK API Access Key"))
	FString APIAccessKey;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK Region ID"))
	FString RegionID;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK Default Language ID: To use if no value is specified"))
	FString LanguageID;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK Default Voice Name: To use if no value is specified"))
	FString VoiceName;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK Profanity Filter"))
	EAzSpeechProfanityFilter ProfanityFilter;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Azure Speech SDK Auto Language Candidates: Used when LanguageID is set to Auto"))
	TArray<FString> AutoLanguageCandidates;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Time limit in seconds to wait for related asynchronous tasks to complete"))
	int32 TimeOutInSeconds;

	/* If enabled, logs will be generated inside Saved/Logs/AzSpeech folder whenever a task fails */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Enable Azure SDK Logs"))
	bool bEnableSDKLogs;

	/* If enabled, synthesizers tasks will generate Viseme data */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Enable Viseme"))
	bool bEnableViseme;

	/* Will print extra debugging informations on log */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Enable Runtime Debug"))
	bool bEnableRuntimeDebug;

protected:
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	virtual void PostLoad() override;
};
