// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AzSpeechSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta = (DisplayName="AzSpeech Settings"))
class AZSPEECH_API UAzSpeechSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	explicit UAzSpeechSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "AzSpeech",
		Meta = (DisplayName = "Azure Speech SDK API Access Key"))
	FString APIAccessKey;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "AzSpeech", Meta = (DisplayName = "Azure Speech SDK Region ID"))
	FString RegionID;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "AzSpeech",
		Meta = (DisplayName = "Azure Speech SDK Default Language ID: To use if no value is specified"))
	FString LanguageID;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "AzSpeech",
		Meta = (DisplayName = "Azure Speech SDK Default Voice Name: To use if no value is specified"))
	FString VoiceName;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "AzSpeech",
		Meta = (DisplayName = "Azure Speech SDK Auto Language Candidates: Used when LanguageID is set to Auto"))
	TArray<FString> AutoLanguageCandidates;

private:
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	FString PreviousLanguage;
#endif // WITH_EDITOR
};