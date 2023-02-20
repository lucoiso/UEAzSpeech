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
	FAzSpeechRunnableBase(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);

	void StartAzSpeechRunnableTask();
	void StopAzSpeechRunnableTask();

	bool IsPendingStop() const;

protected:
	// FRunnable interface
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	// End of FRunnable interface

	typedef FAzSpeechRunnableBase Super;

	UAzSpeechTaskBase* GetOwningTask() const;
	std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig> GetAudioConfig() const;

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

	virtual const bool ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const;

	const bool EnableLogInConfiguration(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const;

	const Microsoft::CognitiveServices::Speech::ProfanityOption GetProfanityFilter() const;

	const FString CancellationReasonToString(const Microsoft::CognitiveServices::Speech::CancellationReason& CancellationReason) const;
	void ProcessCancellationError(const Microsoft::CognitiveServices::Speech::CancellationErrorCode& ErrorCode, const std::string& ErrorDetails) const;

	const EThreadPriority GetCPUThreadPriority() const;
	const float GetThreadUpdateInterval() const;

	static const int64 GetTimeInMilliseconds();
	const int32 GetTimeout() const;

	const FString GetThreadName() const;

private:
	FName ThreadName;

	void StoreThreadInformation();

	bool bStopTask = false;
	TUniquePtr<FRunnableThread> Thread;
	UAzSpeechTaskBase* OwningTask;
	std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig> AudioConfig;
	mutable FCriticalSection Mutex;

#if !UE_BUILD_SHIPPING
protected:
	void PrintDebugInformation(const int64 StartTime, const int64 ActivationDelay, const float SleepTime) const;
#endif
};
