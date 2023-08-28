// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Synthesis/Bases/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeechWavFileSynthesisBase.generated.h"

/**
 *
 */
UCLASS(Abstract, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class AZSPEECH_API UAzSpeechWavFileSynthesisBase : public UAzSpeechSynthesizerTaskBase
{
    GENERATED_BODY()

public:
    /* Task delegate that will be called when completed */
    UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
    FBooleanSynthesisDelegate SynthesisCompleted;

    virtual void Activate() override;
    virtual void SetReadyToDestroy() override;

protected:
    virtual bool StartAzureTaskWork() override;
    virtual void BroadcastFinalResult() override;

    FString FilePath;
    FString FileName;
};
