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
	// START OF DEPRECATED FUNCTIONS
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (DeprecatedFunction="true",
		DeprecationMessage="This function will be replaced soon. Use ConvertFileToSoundWave instead it."))
	static USoundWave* ConvertFileIntoSoundWave(const FString FilePath, const FString FileName)
	{
		return ConvertFileToSoundWave(FilePath, FileName);
	}

	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (DeprecatedFunction="true",
		DeprecationMessage="This function will be replaced soon. Use ConvertFileToSoundWave instead it."))
	static USoundWave* ConvertStreamIntoSoundWave(const TArray<uint8> RawData)
	{
		return ConvertStreamToSoundWave(RawData);
	}
	// END OF DEPRECATED FUNCTIONS

	/* Convert a file into a USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static USoundWave* ConvertFileToSoundWave(const FString FilePath, const FString FileName);

	/* Convert a data stream (TArray<uint8>) into a USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static USoundWave* ConvertStreamToSoundWave(const TArray<uint8> RawData);
};
