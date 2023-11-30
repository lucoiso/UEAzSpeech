// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Runnables/Recognition/Bases/AzSpeechRecognitionRunnableBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_speech_recognizer.h>
THIRD_PARTY_INCLUDES_END

/**
 *
 */
    class FAzSpeechRecognitionRunnable : public FAzSpeechRecognitionRunnableBase
{
public:
    FAzSpeechRecognitionRunnable() = delete;
    FAzSpeechRecognitionRunnable(UAzSpeechTaskBase* const InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);

protected:
    // FRunnable interface
    virtual uint32 Run() override;
    // End of FRunnable interface
};
