// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Runnables/Recognition/Bases/AzSpeechRecognitionRunnableBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_keyword_recognition_model.h>
THIRD_PARTY_INCLUDES_END

/**
 *
 */
    class FAzSpeechKeywordRecognitionRunnable : public FAzSpeechRecognitionRunnableBase
{
public:
    FAzSpeechKeywordRecognitionRunnable() = delete;
    FAzSpeechKeywordRecognitionRunnable(UAzSpeechTaskBase* const InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig, const std::shared_ptr<Microsoft::CognitiveServices::Speech::KeywordRecognitionModel>& InModel = nullptr);

protected:
    // FRunnable interface
    virtual uint32 Run() override;
    // End of FRunnable interface

private:
    std::shared_ptr<Microsoft::CognitiveServices::Speech::KeywordRecognitionModel> Model;
};
