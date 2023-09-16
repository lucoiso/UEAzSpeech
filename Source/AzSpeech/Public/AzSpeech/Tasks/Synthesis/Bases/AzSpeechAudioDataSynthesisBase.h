// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Synthesis/Bases/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeechAudioDataSynthesisBase.generated.h"

/**
 *
 */
UCLASS(Abstract, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class AZSPEECH_API UAzSpeechAudioDataSynthesisBase : public UAzSpeechSynthesizerTaskBase
{
    GENERATED_BODY()

protected:
    virtual bool StartAzureTaskWork() override;

    TWeakObjectPtr<UObject> WorldContextObject;
};
