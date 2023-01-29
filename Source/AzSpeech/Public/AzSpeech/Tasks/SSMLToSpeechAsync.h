// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechSpeechSynthesisBase.h"
#include "SSMLToSpeechAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToSpeechAsync : public UAzSpeechSpeechSynthesisBase
{
	GENERATED_BODY()

public:
	/* Creates a SSML-To-Speech task that will convert your SSML file to speech */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Speech"))
	static USSMLToSpeechAsync* SSMLToSpeech(UObject* WorldContextObject, const FString& SynthesisSSML);
};