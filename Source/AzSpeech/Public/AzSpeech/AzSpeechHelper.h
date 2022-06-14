// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechData.h"
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
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static bool IsAzSpeechDataEmpty(const FAzSpeechData Data);

	/* Helper function to qualify a WAV file path + name into a single string like Full/File/Path/Filename.wav */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (DisplayName = "Qualify WAV File Path"))
	static FString QualifyWAVFileName(const FString& Path, const FString& Name);

	/* Convert file to USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static USoundWave* ConvertFileToSoundWave(const FString& FilePath, const FString& FileName);

	/* Convert data stream (TArray<uint8>) to USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static USoundWave* ConvertStreamToSoundWave(const TArray<uint8> RawData);
};
