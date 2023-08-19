// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechTaskBase.h"
#include "IntentRecognitionAsync.generated.h"

/**
 *
 */
UCLASS(Abstract, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class AZSPEECH_API UIntentRecognitionAsync : public UAzSpeechTaskBase
{
    GENERATED_BODY()

    friend class FAzSpeechIntentRecognitionRunnable;

public:
};
