// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"

/**
 *
 */
class FAzSpeechIntentRecognitionRunnable : public FAzSpeechRunnableBase
{
public:
    FAzSpeechIntentRecognitionRunnable() = delete;

protected:

    // FRunnable interface
    virtual uint32 Run() override;
    virtual void Exit() override;
    // End of FRunnable interface
};
