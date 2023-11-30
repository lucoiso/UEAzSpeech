// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Synthesis/Bases/AzSpeechWavFileSynthesisBase.h"
#include "TextToWavFileAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UTextToWavFileAsync : public UAzSpeechWavFileSynthesisBase
{
    GENERATED_BODY()

public:
    /* Creates a Text-To-WavFile task that will convert your string to a .wav audio file */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Text To .wav File with Default Options"))
    static UTextToWavFileAsync* TextToWavFile_DefaultOptions(UObject* const WorldContextObject, const FString& SynthesisText, const FString& FilePath, const FString& FileName, const FString& Voice = "Default", const FString& Locale = "Default");

    /* Creates a Text-To-WavFile task that will convert your string to a .wav audio file */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Text To .wav File with Custom Options"))
    static UTextToWavFileAsync* TextToWavFile_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText, const FString& FilePath, const FString& FileName);
};
