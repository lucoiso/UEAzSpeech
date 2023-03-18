// Author: Lucas Vilas-Boas
// Year: 2023
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
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Speech", DeprecatedFunction = "true", DeprecationMessage = "Use the version with the new options structure instead"))
	static FORCEINLINE USSMLToSpeechAsync* SSMLToSpeech(UObject* WorldContextObject, const FString& SynthesisSSML)
	{
		return SSMLToSpeechAsync(WorldContextObject, SynthesisSSML, FAzSpeechSettingsOptions());
	}

	/* Creates a SSML-To-Speech task that will convert your SSML file to speech */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Speech Async"))
	static USSMLToSpeechAsync* SSMLToSpeechAsync(UObject* WorldContextObject, const FString& SynthesisSSML, const FAzSpeechSettingsOptions& Options);
};