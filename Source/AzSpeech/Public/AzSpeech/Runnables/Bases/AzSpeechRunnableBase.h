// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <HAL/Runnable.h>

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_embedded_speech_config.h>
#include <speechapi_cxx_hybrid_speech_config.h>
#include <speechapi_cxx_speech_config.h>
#include <speechapi_cxx_audio_config.h>
#include <speechapi_cxx_eventsignal.h>
THIRD_PARTY_INCLUDES_END

class UAzSpeechTaskBase;

/**
 *
 */
class FAzSpeechRunnableBase : public FRunnable
{
public:
    FAzSpeechRunnableBase() = delete;
    FAzSpeechRunnableBase(UAzSpeechTaskBase* const InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);

    ~FAzSpeechRunnableBase();

    void StartAzSpeechRunnableTask();
    void StopAzSpeechRunnableTask();

    bool IsRunning() const;
    bool IsPendingStop() const;

protected:
    // FRunnable interface
    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
    virtual void Exit() override;
    // End of FRunnable interface

    UAzSpeechTaskBase* GetOwningTask() const;
    const std::chrono::seconds GetTaskTimeout() const;
    virtual bool InitializeAzureObject();
    virtual bool CanInitializeTask() const;

    std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig> GetAudioConfig() const;
    std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig> CreateSpeechConfig() const;

    virtual const bool ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const;
    const bool EnableLogInConfiguration(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const;

    void InsertProfanityFilterProperty(const EAzSpeechProfanityFilter Mode, const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const;
    void InsertLanguageIdentificationProperty(const EAzSpeechLanguageIdentificationMode Mode, const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const;

    const FString CancellationReasonToString(const Microsoft::CognitiveServices::Speech::CancellationReason CancellationReason) const;
    void ProcessCancellationError(const Microsoft::CognitiveServices::Speech::CancellationErrorCode ErrorCode, const std::string& ErrorDetails) const;

    const EThreadPriority GetCPUThreadPriority() const;
    const float GetThreadUpdateInterval() const;
    const int32 GetTimeout() const;
    const FString GetThreadName() const;

private:
    FName ThreadName;
    void StoreThreadInformation();

    bool bStopTask = false;
    TUniquePtr<FRunnableThread> Thread;
    TWeakObjectPtr<UAzSpeechTaskBase> OwningTask;
    std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig> AudioConfig;

protected:
    mutable FCriticalSection Mutex;
};
