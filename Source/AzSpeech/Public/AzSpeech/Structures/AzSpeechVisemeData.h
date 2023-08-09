// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeechVisemeData.generated.h"

USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechVisemeData
{
    GENERATED_BODY()

    FAzSpeechVisemeData() = default;

    FAzSpeechVisemeData(const int32 InVisemeID, const int64 InAudioOffsetMilliseconds, const FString& InAnimation) : VisemeID(InVisemeID), AudioOffsetMilliseconds(InAudioOffsetMilliseconds), Animation(InAnimation)
    {
    }

    const bool IsValid() const
    {
        return VisemeID != -1 && AudioOffsetMilliseconds != -1;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
    int32 VisemeID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
    int64 AudioOffsetMilliseconds = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
    FString Animation = FString();
};
