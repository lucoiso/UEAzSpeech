// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/AzSpeechRecognitionRunnable.h"
#include "AzSpeech/Tasks/Bases/AzSpeechRecognizerTaskBase.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include "Async/Async.h"

FAzSpeechRecognitionRunnable::FAzSpeechRecognitionRunnable(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig) : Super(InOwningTask, InAudioConfig)
{
}

const std::shared_ptr<Microsoft::CognitiveServices::Speech::Recognizer> FAzSpeechRecognitionRunnable::GetRecognizer() const
{
	return SpeechRecognizer->shared_from_this();
}

uint32 FAzSpeechRecognitionRunnable::Run()
{
	if (Super::Run() == 0u)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Run returned 0"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return 0u;
	}
	
	if (!SpeechRecognizer)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid recognizer"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return 0u;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = Cast<UAzSpeechRecognizerTaskBase>(OwningTask);

	if (!IsValid(RecognizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return 0u;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Starting recognition. Mode: %s"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), RecognizerTask->IsUsingContinuousRecognition() ? *FString("Continuous") : *FString("Single"));

	if (RecognizerTask->IsUsingContinuousRecognition())
	{
		SpeechRecognizer->StartContinuousRecognitionAsync().wait_for(GetTaskTimeout());
	}
	else
	{
		SpeechRecognizer->RecognizeOnceAsync().wait_for(GetTaskTimeout());
	}

	if (RecognizerTask->RecognitionStarted.IsBound())
	{
		AsyncTask(ENamedThreads::GameThread, [RecognizerTask] 
		{ 
			RecognizerTask->RecognitionStarted.Broadcast();
			RecognizerTask->RecognitionStarted.Clear();
		});
	}

#if !UE_BUILD_SHIPPING
	const UAzSpeechSettings* const Settings = AzSpeech::Internal::GetPluginSettings();
	const bool bEnablePrints = Settings->bEnableDebuggingLogs;
	float InSeconds = 0.f;
#endif

	constexpr float SleepTime = 0.1f;
	while (!IsPendingStop())
	{
#if !UE_BUILD_SHIPPING
		if (bEnablePrints)
		{
			GEngine->AddOnScreenDebugMessage((int32)OwningTask->GetUniqueID(), 5.f, FColor::Yellow, FString::Printf(TEXT("Task: %s (%d). Active time: %f seconds\nCurrent recognized string: %s\nNote: Disable Debugging Logs to avoid this Print"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), InSeconds, *RecognizerTask->GetRecognizedString()));
			InSeconds += SleepTime;
		}
#endif

		FPlatformProcess::Sleep(SleepTime);
	}
	
	return 1u;
}

void FAzSpeechRecognitionRunnable::Stop()
{
	Super::Stop();

	if (SpeechRecognizer)
	{
		SpeechRecognizer->StopContinuousRecognitionAsync().wait_for(GetTaskTimeout());
	}

	SpeechRecognizer = nullptr;
}

void FAzSpeechRecognitionRunnable::ClearSignals()
{
	Super::ClearSignals();

	if (!SpeechRecognizer)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid recognizer"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return;
	}
	
	SignalDisconnecter_T(SpeechRecognizer->Recognizing);
	SignalDisconnecter_T(SpeechRecognizer->Recognized);
}

void FAzSpeechRecognitionRunnable::RemoveBindings()
{
	Super::RemoveBindings();
	
	UAzSpeechRecognizerTaskBase* const RecognizerTask = Cast<UAzSpeechRecognizerTaskBase>(OwningTask);
		
	if (!IsValid(RecognizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return;
	}

	DelegateDisconnecter_T(RecognizerTask->RecognitionStarted);
	DelegateDisconnecter_T(RecognizerTask->RecognitionUpdated);
}

bool FAzSpeechRecognitionRunnable::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) const
{
	if (!Super::ApplySDKSettings(InConfig))
	{
		return false;
	}
	
	if (OwningTask->IsUsingAutoLanguage())
	{
		return true;
	}
	
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Using language: %s"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), *OwningTask->GetLanguageID());

	const std::string UsedLang = TCHAR_TO_UTF8(*OwningTask->GetLanguageID());
	InConfig->SetSpeechRecognitionLanguage(UsedLang);

	return !AzSpeech::Internal::HasEmptyParam(UsedLang);
}

bool FAzSpeechRecognitionRunnable::InitializeAzureObject()
{
	if (!Super::InitializeAzureObject())
	{
		return false;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = Cast<UAzSpeechRecognizerTaskBase>(OwningTask);

	if (!IsValid(RecognizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Creating recognizer object"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

	const auto SpeechConfig = CreateSpeechConfig();

	if (!SpeechConfig)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid speech config"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	ApplySDKSettings(SpeechConfig);

	if (RecognizerTask->IsUsingAutoLanguage())
	{
		const std::vector<std::string> Candidates = AzSpeech::Internal::GetCandidateLanguages(*RecognizerTask->GetTaskName(), RecognizerTask->GetUniqueID(), RecognizerTask->IsUsingContinuousRecognition());
		if (Candidates.empty())
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Task failed. Result: Invalid candidate languages"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
			
			return false;
		}

		SpeechRecognizer = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(SpeechConfig, Microsoft::CognitiveServices::Speech::AutoDetectSourceLanguageConfig::FromLanguages(Candidates), AudioConfig);
	}
	else
	{
		SpeechRecognizer = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(SpeechConfig, AudioConfig);
	}

	return ConnectRecognitionSignals();
}

bool FAzSpeechRecognitionRunnable::ConnectRecognitionSignals()
{
	if (!SpeechRecognizer)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid recognizer"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = Cast<UAzSpeechRecognizerTaskBase>(OwningTask);
	if (!IsValid(RecognizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));	
		return false;
	}
	
	const auto RecognitionUpdate_Lambda = [this, RecognizerTask](const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
	{
		if (!IsValid(RecognizerTask) || !ProcessRecognitionResult(RecognitionEventArgs.Result))
		{
			Stop();
			return;
		}
		
		AsyncTask(ENamedThreads::GameThread, [RecognizerTask, TaskResult = RecognitionEventArgs.Result] { RecognizerTask->OnRecognitionUpdated(TaskResult); });
	};

	SpeechRecognizer->Recognized.Connect(RecognitionUpdate_Lambda);

	if (RecognizerTask->IsUsingContinuousRecognition())
	{
		SpeechRecognizer->Recognizing.Connect(RecognitionUpdate_Lambda);
	}
	
	return true;
}

bool FAzSpeechRecognitionRunnable::ProcessRecognitionResult(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognitionResult>& LastResult)
{
	bool bOutput = true;
	bool bFinishTask = false;

	switch (LastResult->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizingSpeech:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Task running. Reason: RecognizingSpeech"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
			bOutput = true;
			bFinishTask = false;
			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Task completed. Reason: RecognizedSpeech"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
			bOutput = true;
			bFinishTask = true;
			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::NoMatch:
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Task failed. Reason: NoMatch"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
			bOutput = false;
			bFinishTask = true;
			break;

		default:
			break;
	}

	if (LastResult->Reason == Microsoft::CognitiveServices::Speech::ResultReason::Canceled)
	{
		bOutput = false;
		bFinishTask = true;

		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Task failed. Reason: Canceled"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::CancellationDetails::FromResult(LastResult);

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
