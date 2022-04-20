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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTextToWavDelegate, const bool, OutputValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWavToTextDelegate, const FString&, RecognizedString);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTextToStreamDelegate, const TArray<uint8>&, RecognizedStream);

/**
 *
 */

 /* Microsoft Azure informations related to Speech service */
USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechData
{
	GENERATED_USTRUCT_BODY()

public:
	/* API Access Key from Azure Portal - Speech Service panel: Keys and Endpoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech | AzureSettings")
		FString APIAccessKey;

	/* Speech Resource Region ID from Azure Portal - Speech Service panel: Keys and Endpoint -
	IDs List: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech | AzureSettings")
		FString RegionID;

	/* Text/Voice Language ID to set localization -
	IDs List: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech | AzureSettings")
		FString LanguageID;
};
