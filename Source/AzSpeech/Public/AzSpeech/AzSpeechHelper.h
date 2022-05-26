// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AzSpeechHelper.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UAzSpeechHelper final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* Convert file to USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static USoundWave* ConvertFileToSoundWave(const FString FilePath, const FString FileName);

	/* Convert data stream (TArray<uint8>) to USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static USoundWave* ConvertStreamToSoundWave(const TArray<uint8> RawData);
};
