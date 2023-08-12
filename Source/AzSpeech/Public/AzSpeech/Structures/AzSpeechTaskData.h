// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeechTaskData.generated.h"

USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechTaskData
{
    GENERATED_BODY()

    FAzSpeechTaskData() = default;
    FAzSpeechTaskData(const int64 InID, UClass* const InClass) : UniqueID(InID), Class(InClass) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
    int64 UniqueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
    UClass* Class;
};