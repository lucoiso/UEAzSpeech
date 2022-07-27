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
	UAzSpeechSettings() = default;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "AzSpeech",
		Meta = (DisplayName = "Azure Speech SDK API Access Key"))
	FString APIAccessKey;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "AzSpeech", Meta = (DisplayName = "Azure Speech SDK Region ID"))
	FString RegionID;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "AzSpeech",
		Meta = (DisplayName = "Azure Speech SDK Default Language ID: To use if no value is specified"))
	FString LanguageId;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "AzSpeech",
		Meta = (DisplayName = "Azure Speech SDK Default Voice Name: To use if no value is specified"))
	FString VoiceName;
};
