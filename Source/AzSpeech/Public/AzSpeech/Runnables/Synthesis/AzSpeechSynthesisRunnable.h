// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_speech_synthesizer.h>
THIRD_PARTY_INCLUDES_END

/**
 *
 */
    class FAzSpeechSynthesisRunnable : public FAzSpeechRunnableBase
{
public:
    FAzSpeechSynthesisRunnable() = delete;
    FAzSpeechSynthesisRunnable(UAzSpeechTaskBase* const InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);

protected:
    // FRunnable interface
    virtual uint32 Run() override;
    virtual void Exit() override;
    // End of FRunnable interface

protected:
    const bool IsSpeechSynthesizerValid() const;
    class UAzSpeechSynthesizerTaskBase* GetOwningSynthesizerTask() const;

    virtual const bool ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) const override;
    virtual bool InitializeAzureObject() override;

private:
    bool ConnectVisemeSignal();
    bool ConnectSynthesisStartedSignal();
    bool ConnectSynthesisUpdateSignals();
    bool ProcessSynthesisResult(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult);

    const Microsoft::CognitiveServices::Speech::SpeechSynthesisOutputFormat GetOutputFormat() const;

protected:
    bool bFilterVisemeData = false;
    std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesizer> SpeechSynthesizer;
};
