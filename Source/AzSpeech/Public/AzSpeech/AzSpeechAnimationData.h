// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeechAnimationData.generated.h"

USTRUCT(BlueprintType, Category = "AzSpeech")
struct FAzSpeechBlendShapes
{
	GENERATED_USTRUCT_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	TArray<float> Data;
};

USTRUCT(BlueprintType, Category = "AzSpeech")
struct FAzSpeechAnimationData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	int32 FrameIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	TArray<FAzSpeechBlendShapes> BlendShapes;
};