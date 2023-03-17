// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechAudioDataSynthesisBase.h"
#include "SSMLToAudioDataAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToAudioDataAsync : public UAzSpeechAudioDataSynthesisBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FAudioDataSynthesisDelegate SynthesisCompleted;

	/* Creates a SSML-To-AudioData task that will convert your SSML file to a audio data */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Audio Data", DeprecatedFunction = "true", DeprecationMessage = "Use the version with the new options structure instead."))
	static FORCEINLINE USSMLToAudioDataAsync* SSMLToAudioData(UObject* WorldContextObject, const FString& SynthesisSSML)
	{
		return SSMLToAudioDataAsync(WorldContextObject, SynthesisSSML);
	}

	/* Creates a SSML-To-AudioData task that will convert your SSML file to a audio data */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Audio Data Async", AutoCreateRefTerm = "Options"))
	static USSMLToAudioDataAsync* SSMLToAudioDataAsync(UObject* WorldContextObject, const FString& SynthesisSSML, const FAzSpeechSettingsOptions& Options = FAzSpeechSettingsOptions());
	
protected:
	virtual void BroadcastFinalResult() override;
};
