// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Synthesis/Bases/AzSpeechAudioDataSynthesisBase.h"
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
    static USSMLToSoundWaveAsync* SSMLToSoundWave_DefaultOptions(UObject* const WorldContextObject, const FString& SynthesisSSML);

    /* Creates a SSML-To-SoundWave task that will convert your SSML file to a USoundWave */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Sound Wave with Custom Options"))
    static USSMLToSoundWaveAsync* SSMLToSoundWave_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML);

protected:
    virtual void BroadcastFinalResult() override;
};