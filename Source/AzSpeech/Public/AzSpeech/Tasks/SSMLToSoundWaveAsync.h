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
	UFUNCTION(BlueprintCallable, Category = "AzSpeech | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Sound Wave with Default Options"))
	static FORCEINLINE USSMLToSoundWaveAsync* SSMLToSoundWave_DefaultOptions(UObject* WorldContextObject, const FString& SynthesisSSML)
	{
		return SSMLToSoundWave_CustomOptions(WorldContextObject, SynthesisSSML, FAzSpeechSettingsOptions());
	}

	/* Creates a SSML-To-SoundWave task that will convert your SSML file to a USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Sound Wave with Custom Options"))
	static USSMLToSoundWaveAsync* SSMLToSoundWave_CustomOptions(UObject* WorldContextObject, const FString& SynthesisSSML, const FAzSpeechSettingsOptions& Options);

protected:
	virtual void BroadcastFinalResult() override;
};