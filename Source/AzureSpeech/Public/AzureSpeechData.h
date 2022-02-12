// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/AzureSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzureSpeechData.generated.h"
/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceToTextDelegate, const FString&, RecognizedString);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTextToVoiceDelegate, const bool, OutputValue);

/**
 *
 */

USTRUCT(BlueprintType, Category = "AzureSpeech")
struct AZURESPEECH_API FAzureSpeechData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzureSpeech | AzureSettings")
	FString SubscriptionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzureSpeech | AzureSettings")
	FString RegionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzureSpeech | AzureSettings")
	FString LanguageID;
};
