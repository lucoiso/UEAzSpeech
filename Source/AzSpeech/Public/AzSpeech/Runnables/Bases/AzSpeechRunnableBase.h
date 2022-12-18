// Author: Lucas Vilas-Boas
// Year: 2022
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

#if !UE_BUILD_SHIPPING
// Used to print debug informations on screen - only available on non-shipping builds
#include <Engine/Engine.h>
#endif

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

	virtual const bool ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const;
	
	const bool EnableLogInConfiguration(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const;
	
	const Microsoft::CognitiveServices::Speech::ProfanityOption GetProfanityFilter() const;

	const FString CancellationReasonToString(const Microsoft::CognitiveServices::Speech::CancellationReason& CancellationReason) const;
	void ProcessCancellationError(const Microsoft::CognitiveServices::Speech::CancellationErrorCode& ErrorCode, const std::string& ErrorDetails) const;

	const EThreadPriority GetCPUThreadPriority() const;
	const float GetThreadUpdateInterval() const;

	static const int64 GetTimeInMilliseconds();
	const int32 GetTimeout() const;
	
#if !UE_BUILD_SHIPPING
	template<typename TaskTy>
	static void PrintDebugInformation(const TaskTy* Task, const int64 StartTime, const int64 ActivationDelay, const float SleepTime)
	{
		if (!IsValid(Task))
		{
			return;
		}

		const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get();
		if (!IsValid(Settings) || !Settings->bEnableDebuggingLogs)
		{
			return;
		}

		const float InSeconds = (FAzSpeechRunnableBase::GetTimeInMilliseconds() - StartTime) / 1000.f;
		FString SpecificDataStr;
		
		if constexpr (std::is_base_of<TaskTy, UAzSpeechRecognizerTaskBase>())
		{
			SpecificDataStr = FString::Printf(TEXT("Current recognized string: %s"), *Task->GetRecognizedString());
		}
		else if constexpr (std::is_base_of<TaskTy, UAzSpeechSynthesizerTaskBase>())
		{
			SpecificDataStr = FString::Printf(TEXT("Current synthesis buffer size: %d"), Task->GetAudioData().Num());
		}

		GEngine->AddOnScreenDebugMessage((int32)Task->GetUniqueID(), 5.f, FColor::Yellow, FString::Printf(TEXT("Task: %s (%d).\nActivation time: %d milliseconds\nActive time: %f seconds\n%s\nNote: Disable Debugging Logs to avoid this Print"), *Task->GetTaskName(), Task->GetUniqueID(), ActivationDelay, InSeconds, *SpecificDataStr));
	}
#endif

private:
	bool bStopTask = false;
	TUniquePtr<FRunnableThread> Thread;
};
