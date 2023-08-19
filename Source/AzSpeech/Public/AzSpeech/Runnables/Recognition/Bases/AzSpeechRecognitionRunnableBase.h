// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <vector>
#include <string>
#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_speech_recognizer.h>
THIRD_PARTY_INCLUDES_END

/**
 *
 */
    class FAzSpeechRecognitionRunnableBase : public FAzSpeechRunnableBase
{
public:
    FAzSpeechRecognitionRunnableBase() = delete;
    FAzSpeechRecognitionRunnableBase(UAzSpeechTaskBase* const InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);

protected:

    // FRunnable interface
    virtual uint32 Run() override;
    virtual void Exit() override;
    // End of FRunnable interface

protected:
    const bool IsSpeechRecognizerValid() const;
    class UAzSpeechRecognizerTaskBase* GetOwningRecognizerTask() const;

    const std::vector<std::string> GetCandidateLanguages() const;
    const TArray<FString> GetPhraseListFromGroup(const FName& InGroup) const;
    const Microsoft::CognitiveServices::Speech::OutputFormat GetOutputFormat() const;

    virtual const bool ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) const override;
    virtual bool InitializeAzureObject() override;

private:
    bool ConnectRecognitionStartedSignals();
    bool ConnectRecognitionUpdatedSignals();
    bool InsertPhraseList() const;

    bool ProcessRecognitionResult(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognitionResult>& LastResult);

protected:
    std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognizer> SpeechRecognizer;
};
