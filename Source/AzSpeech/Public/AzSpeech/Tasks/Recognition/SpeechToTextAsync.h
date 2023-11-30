// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Recognition/Bases/AzSpeechRecognizerTaskBase.h"
#include "SpeechToTextAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USpeechToTextAsync : public UAzSpeechRecognizerTaskBase
{
    GENERATED_BODY()

public:
    /* Creates a Speech-To-Text task that will convert your speech to string */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Speech to Text with Default Options", AutoCreateRefTerm = "PhraseListGroup"))
    static USpeechToTextAsync* SpeechToText_DefaultOptions(UObject* const WorldContextObject, const FString& Locale = "Default", const FString& AudioInputDeviceID = "Default", const FName& PhraseListGroup = NAME_None);

    /* Creates a Speech-To-Text task that will convert your speech to string */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Speech to Text with Custom Options", AutoCreateRefTerm = "PhraseListGroup"))
    static USpeechToTextAsync* SpeechToText_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechRecognitionOptions& RecognitionOptions, const FString& AudioInputDeviceID = "Default", const FName& PhraseListGroup = NAME_None);

    virtual void Activate() override;

    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    bool IsUsingDefaultAudioInputDevice() const;

protected:
    virtual bool StartAzureTaskWork() override;

private:
    FString AudioInputDeviceID;
};
