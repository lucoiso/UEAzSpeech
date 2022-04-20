// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AzSpeechHelper.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UAzSpeechHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* Convert a file into a USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
		static USoundWave* ConvertFileIntoSoundWave(const FString FilePath, const FString ObjectName = "Transient_AzSpeechSoundWave");

	/* Convert a data stream (TArray<uint8>) into a USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
		static USoundWave* ConvertStreamIntoSoundWave(TArray<uint8> RawData, const FString ObjectName = "Transient_AzSpeechSoundWave");
};
