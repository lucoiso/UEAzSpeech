// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Synthesis/Bases/AzSpeechAudioDataSynthesisBase.h"
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

#if WITH_EDITOR
    static USSMLToAudioDataAsync* EditorTask(const FString& SynthesisSSML);
#endif

    /* Creates a SSML-To-AudioData task that will convert your SSML file to a audio data */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Audio Data with Default Options"))
    static USSMLToAudioDataAsync* SSMLToAudioData_DefaultOptions(UObject* const WorldContextObject, const FString& SynthesisSSML);

    /* Creates a SSML-To-AudioData task that will convert your SSML file to a audio data */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Audio Data with Custom Options"))
    static USSMLToAudioDataAsync* SSMLToAudioData_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML);

protected:
    virtual void BroadcastFinalResult() override;
};
