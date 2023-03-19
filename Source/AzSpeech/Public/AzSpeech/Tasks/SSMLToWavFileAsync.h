// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechWavFileSynthesisBase.h"
#include "SSMLToWavFileAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToWavFileAsync : public UAzSpeechWavFileSynthesisBase
{
	GENERATED_BODY()

public:
	/* Creates a Text-To-WavFile task that will convert your string to a .wav audio file */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To .wav File with Default Options"))
	static FORCEINLINE USSMLToWavFileAsync* SSMLToWavFile_DefaultOptions(UObject* WorldContextObject, const FString& SynthesisSSML, const FString& FilePath, const FString& FileName)
	{
		return SSMLToWavFile_CustomOptions(WorldContextObject, SynthesisSSML, FilePath, FileName, FAzSpeechSettingsOptions());
	}

	/* Creates a Text-To-WavFile task that will convert your string to a .wav audio file */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To .wav File with Custom Options"))
	static USSMLToWavFileAsync* SSMLToWavFile_CustomOptions(UObject* WorldContextObject, const FString& SynthesisSSML, const FString& FilePath, const FString& FileName, const FAzSpeechSettingsOptions& Options);
};
