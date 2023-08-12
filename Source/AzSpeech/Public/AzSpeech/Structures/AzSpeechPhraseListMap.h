// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeechPhraseListMap.generated.h"

USTRUCT(BlueprintType, Category = "AzSpeech")
struct AZSPEECH_API FAzSpeechPhraseListMap
{
    GENERATED_BODY()

    /* The name of this phrase list data group */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
    FName GroupName = NAME_None;

    /* Array of phrase lists that will be used to improve recognition accuracy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech", Meta = (DisplayName = "Phrase List"))
    TArray<FString> Data;

    bool operator==(const FAzSpeechPhraseListMap& Rhs) const
    {
        return GroupName == Rhs.GroupName || Data == Rhs.Data;
    }
};