// Author: Lucas Vilas-Boas
// Year: 2022
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
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To .wav File"))
	static USSMLToWavFileAsync* SSMLToWavFile(UObject* WorldContextObject, const FString& SynthesisSSML, const FString& FilePath, const FString& FileName);
};
