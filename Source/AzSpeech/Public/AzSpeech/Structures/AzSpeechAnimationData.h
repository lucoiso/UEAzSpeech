// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeechAnimationData.generated.h"

USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechBlendShapes
{
    GENERATED_BODY()

    FAzSpeechBlendShapes() = default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
    TArray<float> Data;
};

USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechAnimationData
{
    GENERATED_BODY()

    FAzSpeechAnimationData() = default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
    int32 FrameIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
    TArray<FAzSpeechBlendShapes> BlendShapes;
};