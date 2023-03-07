// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"
#include "AzSpeech/Tasks/Bases/AzSpeechTaskBase.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"
#include <HAL/ThreadManager.h>
#include <Misc/FileHelper.h>
#include <Async/Async.h>
#include <chrono>

#if ENGINE_MAJOR_VERSION < 5
#include <HAL/PlatformFilemanager.h>
#else
#include <HAL/PlatformFileManager.h>
#endif

#if !UE_BUILD_SHIPPING
// Used to print debug informations on screen - only available on non-shipping builds
#include <Engine/Engine.h>
#include "AzSpeech/Tasks/Bases/AzSpeechRecognizerTaskBase.h"
#include "AzSpeech/Tasks/Bases/AzSpeechSynthesizerTaskBase.h"
#endif

FAzSpeechRunnableBase::FAzSpeechRunnableBase(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig) : OwningTask(InOwningTask), AudioConfig(InAudioConfig)
{
}

void FAzSpeechRunnableBase::StartAzSpeechRunnableTask()
{
	Thread.Reset(FRunnableThread::Create(this, *FString::Printf(TEXT("AzSpeech_%s_%d"), *GetOwningTask()->GetTaskName(), GetOwningTask()->GetUniqueID()), 0u, GetCPUThreadPriority()));
}

void FAzSpeechRunnableBase::StopAzSpeechRunnableTask()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Setting runnable work as pending stop"), *GetThreadName(), *FString(__func__));
	bStopTask = true;
}

bool FAzSpeechRunnableBase::IsPendingStop() const
{
	return bStopTask;
}

bool FAzSpeechRunnableBase::Init()
{
	StoreThreadInformation();

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Initializing runnable thread"), *GetThreadName(), *FString(__func__));
	
	return CanInitializeTask();
}

uint32 FAzSpeechRunnableBase::Run()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Running runnable thread work"), *GetThreadName(), *FString(__func__));
	
	return InitializeAzureObject() ? 1u : 0u;
}

void FAzSpeechRunnableBase::Stop()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Stopping runnable thread work"), *GetThreadName(), *FString(__func__));
}

void FAzSpeechRunnableBase::Exit()
{
	FScopeLock Lock_Runnable(&Mutex);
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Exiting thread"), *GetThreadName(), *FString(__func__));

	ClearSignals();

	if (UAzSpeechTaskStatus::IsTaskActive(GetOwningTask()))
	{
		FScopeLock Lock_Task(&GetOwningTask()->Mutex);
		AsyncTask(ENamedThreads::GameThread, [this] { GetOwningTask()->BroadcastFinalResult(); });
	}

	RemoveBindings();

	if (UAzSpeechTaskStatus::IsTaskStillValid(GetOwningTask()) && !UAzSpeechTaskStatus::IsTaskReadyToDestroy(GetOwningTask()))
	{
		GetOwningTask()->SetReadyToDestroy();
	}
}

UAzSpeechTaskBase* FAzSpeechRunnableBase::GetOwningTask() const
{
	if (!OwningTask)
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function %s; Message: Tried to get an invalid AzSpeech task."), *GetThreadName(), *FString(__func__));
	}

	return OwningTask;
}

std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig> FAzSpeechRunnableBase::GetAudioConfig() const
{
	if (!AudioConfig)
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function %s; Message: Tried to get an invalid Audio Config."), *GetThreadName(), *FString(__func__));
	}

	return AudioConfig;
}

bool FAzSpeechRunnableBase::InitializeAzureObject()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Initializing Azure Object"), *GetThreadName(), *FString(__func__));
	
	return true;
}

bool FAzSpeechRunnableBase::CanInitializeTask() const
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Checking if can initialize task in current context"), *GetThreadName(), *FString(__func__));
	
	bool bOutput = true;

	if (!UAzSpeechSettings::CheckAzSpeechSettings())
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Failed to initialize task due to invalid settings"), *GetThreadName(), *FString(__func__));

		bOutput = false;
	}

	if (!IsValid(GetOwningTask()))
	{
		bOutput = false;
	}

	return bOutput;
}

void FAzSpeechRunnableBase::ClearSignals()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Disconnecting Azure SDK recognition signals"), *GetThreadName(), *FString(__func__));
}

void FAzSpeechRunnableBase::RemoveBindings()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Removing existing delegate bindings"), *GetThreadName(), *FString(__func__));
}

std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig> FAzSpeechRunnableBase::CreateSpeechConfig() const
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Creating Azure SDK speech config"), *GetThreadName(), *FString(__func__));

	const auto Settings = UAzSpeechSettings::GetAzSpeechKeys();
	const auto SpeechConfig = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));

	if (!SpeechConfig)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Failed to create speech configuration"), *GetThreadName(), *FString(__func__));
		
		return nullptr;
	}

	return SpeechConfig;
}

const std::chrono::seconds FAzSpeechRunnableBase::GetTaskTimeout() const
{
	return std::chrono::seconds(GetTimeout());
}

const bool FAzSpeechRunnableBase::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const
{
	FScopeLock Lock(&Mutex);

	if (!InSpeechConfig)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Invalid speech config"), *GetThreadName(), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Applying Azure SDK Settings"), *GetThreadName(), *FString(__func__));

	EnableLogInConfiguration(InSpeechConfig);

	InSpeechConfig->SetProfanity(GetProfanityFilter());

	if (GetOwningTask()->IsUsingAutoLanguage())
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Using auto language identification"), *GetThreadName(), *FString(__func__));
		
		InSpeechConfig->SetProperty(Microsoft::CognitiveServices::Speech::PropertyId::SpeechServiceConnection_LanguageIdMode, "Continuous");
	}

	return true;
}

const bool FAzSpeechRunnableBase::EnableLogInConfiguration(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const
{
	FScopeLock Lock(&Mutex);

	if (!UAzSpeechSettings::Get()->bEnableSDKLogs)
	{
		return true;
	}

#if PLATFORM_ANDROID || UE_BUILD_SHIPPING
	return true;
#else

	if (!InSpeechConfig)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Invalid speech config"), *GetThreadName(), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Enabling Azure SDK log"), *GetThreadName(), *FString(__func__));

	if (FString AzSpeechLogPath = UAzSpeechHelper::GetAzSpeechLogsBaseDir(); IFileManager::Get().MakeDirectory(*AzSpeechLogPath, true))
	{
		const FString LogFilename = "UEAzSpeech " + FDateTime::Now().ToString() + ".log";
		AzSpeechLogPath = FPaths::Combine(AzSpeechLogPath, LogFilename);
		FPaths::NormalizeFilename(AzSpeechLogPath);

		if (FFileHelper::SaveStringToFile(FString(), *AzSpeechLogPath))
		{
			InSpeechConfig->SetProperty(Microsoft::CognitiveServices::Speech::PropertyId::Speech_LogFilename, TCHAR_TO_UTF8(*AzSpeechLogPath));
			return true;
		}
	}

	return false;
#endif
}

const Microsoft::CognitiveServices::Speech::ProfanityOption FAzSpeechRunnableBase::GetProfanityFilter() const
{
	if (const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get())
	{
		switch (Settings->ProfanityFilter)
		{
			case EAzSpeechProfanityFilter::Raw:
				return Microsoft::CognitiveServices::Speech::ProfanityOption::Raw;

			case EAzSpeechProfanityFilter::Removed:
				return Microsoft::CognitiveServices::Speech::ProfanityOption::Removed;

			case EAzSpeechProfanityFilter::Masked:
				return Microsoft::CognitiveServices::Speech::ProfanityOption::Masked;

			default: break;
		}
	}

	return Microsoft::CognitiveServices::Speech::ProfanityOption::Raw;
}

const FString FAzSpeechRunnableBase::CancellationReasonToString(const Microsoft::CognitiveServices::Speech::CancellationReason& CancellationReason) const
{
	switch (CancellationReason)
	{
		case Microsoft::CognitiveServices::Speech::CancellationReason::Error:
			return FString("Error");

		case Microsoft::CognitiveServices::Speech::CancellationReason::EndOfStream:
			return FString("EndOfStream");

		case Microsoft::CognitiveServices::Speech::CancellationReason::CancelledByUser:
			return FString("CancelledByUser");

		default:
			return FString("Undefined");
	}
}

void FAzSpeechRunnableBase::ProcessCancellationError(const Microsoft::CognitiveServices::Speech::CancellationErrorCode& ErrorCode, const std::string& ErrorDetails) const
{
	FString ErrorCodeStr;
	switch (ErrorCode)
	{
		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::NoError:
			ErrorCodeStr = "Error";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::AuthenticationFailure:
			ErrorCodeStr = "AuthenticationFailure";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::BadRequest:
			ErrorCodeStr = "BadRequest";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::TooManyRequests:
			ErrorCodeStr = "TooManyRequests";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::Forbidden:
			ErrorCodeStr = "Forbidden";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::ConnectionFailure:
			ErrorCodeStr = "ConnectionFailure";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::ServiceTimeout:
			ErrorCodeStr = "ServiceTimeout";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::ServiceError:
			ErrorCodeStr = "ServiceError";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::ServiceUnavailable:
			ErrorCodeStr = "ServiceUnavailable";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::RuntimeError:
			ErrorCodeStr = "RuntimeError";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::ServiceRedirectTemporary:
			ErrorCodeStr = "ServiceRedirectTemporary";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::ServiceRedirectPermanent:
			ErrorCodeStr = "ServiceRedirectPermanent";
			break;

		case Microsoft::CognitiveServices::Speech::CancellationErrorCode::EmbeddedModelError:
			ErrorCodeStr = "EmbeddedModelError";
			break;

		default:
			ErrorCodeStr = "Undefined";
			break;
	}

	UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Error code: %s"), *GetThreadName(), *FString(__func__), *ErrorCodeStr);
	
	UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Error Details: %s"), *GetThreadName(), *FString(__func__), *UTF8_TO_TCHAR(ErrorDetails.c_str()));
	
	UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Log generated in directory: %s"), *GetThreadName(), *FString(__func__), *UAzSpeechHelper::GetAzSpeechLogsBaseDir());
}

const EThreadPriority FAzSpeechRunnableBase::GetCPUThreadPriority() const
{
	if (const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get())
	{
		switch (Settings->TasksThreadPriority)
		{
			case EAzSpeechThreadPriority::Lowest:
				return EThreadPriority::TPri_Lowest;

			case EAzSpeechThreadPriority::BelowNormal:
				return EThreadPriority::TPri_Lowest;

			case EAzSpeechThreadPriority::Normal:
				return EThreadPriority::TPri_Lowest;

			case EAzSpeechThreadPriority::AboveNormal:
				return EThreadPriority::TPri_Lowest;

			case EAzSpeechThreadPriority::Highest:
				return EThreadPriority::TPri_Lowest;

			default:
				break;
		}
	}

	return EThreadPriority::TPri_Normal;
}

const float FAzSpeechRunnableBase::GetThreadUpdateInterval() const
{
	if (const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get())
	{
		return Settings->ThreadUpdateInterval <= 0.f ? 0.1f : Settings->ThreadUpdateInterval;
	}

	return 0.1f;
}

const int64 FAzSpeechRunnableBase::GetTimeInMilliseconds()
{
	const auto CurrentTime = std::chrono::system_clock::now();
	const auto TimeSinceEpoch = CurrentTime.time_since_epoch();
	const auto CurrentTimeInMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(TimeSinceEpoch);

	return static_cast<int64>(CurrentTimeInMilliseconds.count());
}

const int32 FAzSpeechRunnableBase::GetTimeout() const
{
	if (const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get())
	{
		return Settings->TimeOutInSeconds <= 0.f ? 15.f : Settings->TimeOutInSeconds;
	}

	return 15.f;
}

const FString FAzSpeechRunnableBase::GetThreadName() const
{
	return ThreadName.ToString();
}

void FAzSpeechRunnableBase::StoreThreadInformation()
{
	const FString& ThreadNameRef = FThreadManager::Get().GetThreadName(FPlatformTLS::GetCurrentThreadId());
	ThreadName = *ThreadNameRef;
}

#if !UE_BUILD_SHIPPING
void FAzSpeechRunnableBase::PrintDebugInformation(const int64 StartTime, const int64 ActivationDelay, const float SleepTime) const
{
	FScopeLock Lock(&Mutex);

	UAzSpeechTaskBase* const Task = GetOwningTask();
	if (!UAzSpeechTaskStatus::IsTaskStillValid(Task) || !UAzSpeechSettings::Get()->bEnableDebuggingLogs)
	{
		return;
	}

	const float InSeconds = (FAzSpeechRunnableBase::GetTimeInMilliseconds() - StartTime) / 1000.f;
	FString SpecificDataStr;

	if (Task->GetClass()->IsChildOf<UAzSpeechRecognizerTaskBase>())
	{
		SpecificDataStr = FString::Printf(TEXT("Current recognized string: %s"), *Cast<UAzSpeechRecognizerTaskBase>(Task)->GetRecognizedString());
	}
	else if (Task->GetClass()->IsChildOf<UAzSpeechSynthesizerTaskBase>())
	{
		SpecificDataStr = FString::Printf(TEXT("Current synthesis buffer allocated size: %d"), Cast<UAzSpeechSynthesizerTaskBase>(Task)->GetAudioData().GetAllocatedSize());
	}

	GEngine->AddOnScreenDebugMessage((int32)Task->GetUniqueID(), 5.f, FColor::Yellow, FString::Printf(TEXT("Task: %s (%d).\nActivation time: %d milliseconds\nActive time: %f seconds\n%s\nNote: Disable Debugging Logs to avoid this Print"), *Task->GetTaskName(), Task->GetUniqueID(), ActivationDelay, InSeconds, *SpecificDataStr));
}
#endif