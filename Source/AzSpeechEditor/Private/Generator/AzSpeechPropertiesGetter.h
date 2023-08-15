// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>
#include "AzSpeechPropertiesGetter.generated.h"

DECLARE_DELEGATE_OneParam(FAvailableVoicesUpdated, TArray<FString>);
DECLARE_DELEGATE_OneParam(FAudioDataGenerated, TArray<uint8>);

UCLASS(MinimalAPI, NotBlueprintable, NotPlaceable, Category = "Implementation")
class UAzSpeechPropertiesGetter : public UObject
{
    GENERATED_BODY()

public:
    explicit UAzSpeechPropertiesGetter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    FAvailableVoicesUpdated OnAvailableVoicesUpdated;
    FAudioDataGenerated OnAudioDataGenerated;

    UFUNCTION()
    void OnAvailableVoicesChanged(const TArray<FString>& Voices);

    UFUNCTION()
    void SynthesisCompleted(const TArray<uint8>& AudioData);

    UFUNCTION()
    void TaskFail();

    void Destroy();
};