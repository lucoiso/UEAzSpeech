// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechData.generated.h"
/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceToTextDelegate, const FString&, RecognizedString);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTextToVoiceDelegate, const bool, OutputValue);

/**
 *
 */

/* Microsoft Azure informations related to Speech service */
USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech | AzureSettings")
	FString SubscriptionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech | AzureSettings")
	FString RegionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech | AzureSettings")
	FString LanguageID;
};
