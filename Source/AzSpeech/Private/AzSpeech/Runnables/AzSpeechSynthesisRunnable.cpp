// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/AzSpeechSynthesisRunnable.h"
#include "AzSpeech/Tasks/Bases/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include "Async/Async.h"

FAzSpeechSynthesisRunnable::FAzSpeechSynthesisRunnable(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig) : Super(InOwningTask, InAudioConfig)
{
}

uint32 FAzSpeechSynthesisRunnable::Run()
{
	if (Super::Run() == 0u)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Run returned 0"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return 0u;
	}
	
	if (!SpeechSynthesizer)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid synthesizer"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return 0u;
	}

	UAzSpeechSynthesizerTaskBase* const SynthesizerTask = Cast<UAzSpeechSynthesizerTaskBase>(OwningTask);

	if (!UAzSpeechTaskBase::IsTaskStillValid(SynthesizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return 0u;
	}

	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Using text: %s"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), *SynthesizerTask->GetSynthesisText());

	const std::string SynthesisStr = TCHAR_TO_UTF8(*SynthesizerTask->GetSynthesisText());
	std::future<std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>> Future;
	if (SynthesizerTask->IsSSMLBased())
	{
		Future = SpeechSynthesizer->StartSpeakingSsmlAsync(SynthesisStr);
	}
	else
	{
		Future = SpeechSynthesizer->StartSpeakingTextAsync(SynthesisStr);
	}

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Starting synthesis."), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
	Future.wait_for(GetTaskTimeout());

#if !UE_BUILD_SHIPPING
	const UAzSpeechSettings* const Settings = AzSpeech::Internal::GetPluginSettings();
	const bool bEnablePrints = Settings->bEnableDebuggingLogs;
	float InSeconds = 0.f;
#endif

	const float SleepTime = AzSpeech::Internal::GetThreadUpdateInterval();
	while (!IsPendingStop())
	{
#if !UE_BUILD_SHIPPING
		if (bEnablePrints)
		{
			GEngine->AddOnScreenDebugMessage((int32)OwningTask->GetUniqueID(), 5.f, FColor::Yellow, FString::Printf(TEXT("Task: %s (%d). Active time: %f seconds\nCurrent synthesis buffer size: %d\nNote: Disable Debugging Logs to avoid this Print"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), InSeconds, SynthesizerTask->GetAudioData().Num()));
			InSeconds += SleepTime;
		}
#endif

		FPlatformProcess::Sleep(SleepTime);
	}

	return 1u;
}

void FAzSpeechSynthesisRunnable::Exit()
{
	Super::Exit();
	
	if (SpeechSynthesizer)
	{
		SpeechSynthesizer->StopSpeakingAsync().wait_for(GetTaskTimeout());
	}

	SpeechSynthesizer = nullptr;
}

void FAzSpeechSynthesisRunnable::ClearSignals()
{
	Super::ClearSignals();
	
	if (!SpeechSynthesizer)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid synthesizer"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return;
	}

	SignalDisconnecter_T(SpeechSynthesizer->VisemeReceived);
	SignalDisconnecter_T(SpeechSynthesizer->Synthesizing);
	SignalDisconnecter_T(SpeechSynthesizer->SynthesisStarted);
	SignalDisconnecter_T(SpeechSynthesizer->SynthesisCompleted);
	SignalDisconnecter_T(SpeechSynthesizer->SynthesisCanceled);
}

void FAzSpeechSynthesisRunnable::RemoveBindings()
{
	Super::RemoveBindings();

	UAzSpeechSynthesizerTaskBase* const SynthesizerTask = Cast<UAzSpeechSynthesizerTaskBase>(OwningTask);

	if (!IsValid(SynthesizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return;
	}

	DelegateDisconnecter_T(SynthesizerTask->VisemeReceived);
	DelegateDisconnecter_T(SynthesizerTask->SynthesisUpdated);
	DelegateDisconnecter_T(SynthesizerTask->SynthesisStarted);
}

bool FAzSpeechSynthesisRunnable::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) const
{
	if (!Super::ApplySDKSettings(InConfig))
	{
		return false;
	}

	UAzSpeechSynthesizerTaskBase* const SynthesizerTask = Cast<UAzSpeechSynthesizerTaskBase>(OwningTask);
	
	if (!IsValid(SynthesizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	InConfig->SetProperty("SpeechSynthesis_KeepConnectionAfterStopping", "false");

	if (SynthesizerTask->IsUsingAutoLanguage())
	{
		return true;
	}

	const std::string UsedLang = TCHAR_TO_UTF8(*SynthesizerTask->GetLanguageID());
	const std::string UsedVoice = TCHAR_TO_UTF8(*SynthesizerTask->GetVoiceName());

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Using language: %s"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), *SynthesizerTask->GetLanguageID());
	InConfig->SetSpeechSynthesisLanguage(UsedLang);

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Using voice: %s"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), *SynthesizerTask->GetVoiceName());
	InConfig->SetSpeechSynthesisVoiceName(UsedVoice);

	return true;
}

bool FAzSpeechSynthesisRunnable::InitializeAzureObject()
{
	if (!Super::InitializeAzureObject())
	{
		return false;
	}
	
	UAzSpeechSynthesizerTaskBase* const SynthesizerTask = Cast<UAzSpeechSynthesizerTaskBase>(OwningTask);

	if (!IsValid(SynthesizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Creating synthesizer object"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

	const auto SpeechConfig = CreateSpeechConfig();

	if (!SpeechConfig)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid speech config"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));	
		return false;
	}
	
	ApplySDKSettings(SpeechConfig);

	if (SynthesizerTask->IsUsingAutoLanguage())
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Initializing auto language detection"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

		SpeechSynthesizer = Microsoft::CognitiveServices::Speech::SpeechSynthesizer::FromConfig(SpeechConfig, Microsoft::CognitiveServices::Speech::AutoDetectSourceLanguageConfig::FromOpenRange(), AudioConfig);
	}
	else
	{
		SpeechSynthesizer = Microsoft::CognitiveServices::Speech::SpeechSynthesizer::FromConfig(SpeechConfig, AudioConfig);
	}

	return ConnectSynthesisSignals();
}

bool FAzSpeechSynthesisRunnable::ConnectSynthesisSignals()
{
	if (!SpeechSynthesizer)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid synthesizer"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	UAzSpeechSynthesizerTaskBase* const SynthesizerTask = Cast<UAzSpeechSynthesizerTaskBase>(OwningTask);
	if (!IsValid(SynthesizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	if (AzSpeech::Internal::GetPluginSettings()->bEnableViseme)
	{
		SpeechSynthesizer->VisemeReceived.Connect([this, SynthesizerTask](const Microsoft::CognitiveServices::Speech::SpeechSynthesisVisemeEventArgs& VisemeEventArgs)
		{
			if (!IsValid(SynthesizerTask))
			{
				StopAzSpeechRunnableTask();
				return;
			}

			FAzSpeechVisemeData LastVisemeData;
			LastVisemeData.VisemeID = VisemeEventArgs.VisemeId;
			LastVisemeData.AudioOffsetMilliseconds = VisemeEventArgs.AudioOffset / 10000;
			LastVisemeData.Animation = UTF8_TO_TCHAR(VisemeEventArgs.Animation.c_str());

			AsyncTask(ENamedThreads::GameThread, [SynthesizerTask, LastVisemeData] { SynthesizerTask->OnVisemeReceived(LastVisemeData); });
		});
	}

	const auto SynthesisUpdate_Lambda = [this, SynthesizerTask](const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
	{
		if (!IsValid(SynthesizerTask) || !ProcessSynthesisResult(SynthesisEventArgs.Result))
		{
			StopAzSpeechRunnableTask();
			return;
		}
		
		AsyncTask(ENamedThreads::GameThread, [SynthesizerTask, Result = SynthesisEventArgs.Result] { SynthesizerTask->OnSynthesisUpdate(Result); });
	};

	SpeechSynthesizer->Synthesizing.Connect(SynthesisUpdate_Lambda);
	SpeechSynthesizer->SynthesisCompleted.Connect(SynthesisUpdate_Lambda);
	SpeechSynthesizer->SynthesisCanceled.Connect(SynthesisUpdate_Lambda);
	
	const auto SynthesisStarted_Lambda = [this, SynthesizerTask]([[maybe_unused]] const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
	{
		if (!IsValid(SynthesizerTask))
		{
			StopAzSpeechRunnableTask();
			return;
		}

		if (SynthesizerTask->SynthesisStarted.IsBound())
		{
			AsyncTask(ENamedThreads::GameThread, [SynthesizerTask] { SynthesizerTask->SynthesisStarted.Broadcast(); });
		}

		SignalDisconnecter_T(SpeechSynthesizer->SynthesisStarted);
	};

	SpeechSynthesizer->SynthesisStarted.Connect(SynthesisStarted_Lambda);		
	
	return true;
}

bool FAzSpeechSynthesisRunnable::ProcessSynthesisResult(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult)
{
	bool bOutput = true;
	bool bFinishTask = false;

	switch (LastResult->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudio:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Task running. Reason: SynthesizingAudio"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
			bOutput = true;
			bFinishTask = false;
			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioCompleted:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Task completed. Reason: SynthesizingAudioCompleted"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
			bOutput = true;
			bFinishTask = true;
			break;
			
		case Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioStarted:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Task started. Reason: SynthesizingAudioStarted"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
			bOutput = true;
			bFinishTask = false;
			break;

		default:
			break;
	}

	if (LastResult->Reason == Microsoft::CognitiveServices::Speech::ResultReason::Canceled)
	{
		bOutput = false;
		bFinishTask = true;
		
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Task failed. Reason: Canceled"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		
		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::SpeechSynthesisCancellationDetails::FromResult(LastResult);

		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Cancellation Reason: %s"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

		if (CancellationDetails->Reason == Microsoft::CognitiveServices::Speech::CancellationReason::Error)
		{
			ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
		}		
	}

	if (bFinishTask)
	{
		StopAzSpeechRunnableTask();
	}

	return bOutput;
}
