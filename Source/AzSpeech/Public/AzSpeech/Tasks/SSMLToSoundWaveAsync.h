// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechAudioDataSynthesisBase.h"
#include "SSMLToSoundWaveAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToSoundWaveAsync : public UAzSpeechAudioDataSynthesisBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FSoundWaveSynthesisDelegate SynthesisCompleted;

	/* Creates a SSML-To-SoundWave task that will convert your SSML file to a USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Sound Wave", DeprecatedFunction = "true", DeprecationMessage = "Use the version with the new options structure instead."))
	static FORCEINLINE USSMLToSoundWaveAsync* SSMLToSoundWave(UObject* WorldContextObject, const FString& SynthesisSSML)
	{
		return SSMLToSoundWaveAsync(WorldContextObject, SynthesisSSML, FAzSpeechSettingsOptions());
	}

	/* Creates a SSML-To-SoundWave task that will convert your SSML file to a USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Sound Wave Async"))
	static USSMLToSoundWaveAsync* SSMLToSoundWaveAsync(UObject* WorldContextObject, const FString& SynthesisSSML, const FAzSpeechSettingsOptions& Options);

protected:
	virtual void BroadcastFinalResult() override;
};