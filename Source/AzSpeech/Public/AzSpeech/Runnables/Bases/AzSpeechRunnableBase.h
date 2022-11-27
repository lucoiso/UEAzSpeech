// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

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
	FAzSpeechRunnableBase(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);

	void StartAzSpeechRunnableTask();
	void StopAzSpeechRunnableTask();

	bool IsPendingStop() const;

	// FRunnable interface
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	// End of FRunnable interface
	
protected:
	typedef FAzSpeechRunnableBase Super;

	UAzSpeechTaskBase* OwningTask;
	std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig> AudioConfig;
	
	virtual bool InitializeAzureObject();
	virtual bool CanInitializeTask();

	virtual void ClearSignals();
	virtual void RemoveBindings();

	template<typename SignalTy>
	void SignalDisconnecter_T(SignalTy& Signal)
	{
		if (Signal.IsConnected())
		{
			Signal.DisconnectAll();
		}
	};

	template<typename DelegateTy>
	void DelegateDisconnecter_T(DelegateTy& Delegate)
	{
		if (Delegate.IsBound())
		{
			Delegate.Clear();
		}		
	};
	
	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig> CreateSpeechConfig() const;
	
	const std::chrono::seconds GetTaskTimeout() const;

	virtual bool ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const;
	bool EnableLogInConfiguration(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const;	

	const FString CancellationReasonToString(const Microsoft::CognitiveServices::Speech::CancellationReason& CancellationReason) const;
	void ProcessCancellationError(const Microsoft::CognitiveServices::Speech::CancellationErrorCode& ErrorCode, const std::string& ErrorDetails) const;

private:
	bool bStopTask = false;
	TUniquePtr<FRunnableThread> Thread;
};
