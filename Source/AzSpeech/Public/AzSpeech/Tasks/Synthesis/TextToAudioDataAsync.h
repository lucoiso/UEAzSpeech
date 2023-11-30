// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Synthesis/Bases/AzSpeechAudioDataSynthesisBase.h"
#include "TextToAudioDataAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UTextToAudioDataAsync : public UAzSpeechAudioDataSynthesisBase
{
    GENERATED_BODY()

public:
    /* Task delegate that will be called when completed */
    UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
    FAudioDataSynthesisDelegate SynthesisCompleted;

#if WITH_EDITOR
    static UTextToAudioDataAsync* EditorTask(const FString& SynthesisText, const FString& Voice, const FString& Locale);
#endif

    /* Creates a Text-To-AudioData task that will convert your text to a audio data stream */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Text To Audio Data with Default Options"))
    static UTextToAudioDataAsync* TextToAudioData_DefaultOptions(UObject* const WorldContextObject, const FString& SynthesisText, const FString& Voice = "Default", const FString& Locale = "Default");

    /* Creates a Text-To-AudioData task that will convert your text to a audio data stream */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Text To Audio Data with Custom Options"))
    static UTextToAudioDataAsync* TextToAudioData_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText);

protected:
    virtual void BroadcastFinalResult() override;
};
