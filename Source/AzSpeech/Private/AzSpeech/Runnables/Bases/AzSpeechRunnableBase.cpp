// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"
#include "AzSpeech/Tasks/Bases/AzSpeechTaskBase.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"
#include <HAL/ThreadManager.h>
#include <Misc/FileHelper.h>
#include <Misc/Paths.h>
#include <Misc/ScopeTryLock.h>
#include <Async/Async.h>

#if ENGINE_MAJOR_VERSION < 5
#include <HAL/PlatformFilemanager.h>
#else
#include <HAL/PlatformFileManager.h>
#endif

FAzSpeechRunnableBase::FAzSpeechRunnableBase(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig) : OwningTask(InOwningTask), AudioConfig(InAudioConfig)
{
}

void FAzSpeechRunnableBase::StartAzSpeechRunnableTask()
{
	Thread.Reset(FRunnableThread::Create(this, *FString::Printf(TEXT("AzSpeech_%s_%d"), *GetOwningTask()->GetTaskName().ToString(), GetOwningTask()->GetUniqueID()), 0u, GetCPUThreadPriority()));
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
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Exiting thread"), *GetThreadName(), *FString(__func__));

	UAzSpeechTaskBase* const Task = GetOwningTask();
	if (!Task)
	{
		return;
	}

	AsyncTask(ENamedThreads::GameThread,
		[Task]
		{
			FScopeTryLock Lock(&Task->Mutex);

			if (Lock.IsLocked())
			{
				return;
			}

			Task->BroadcastFinalResult();
			Task->SetReadyToDestroy();
		}
	);
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
	
	const UAzSpeechTaskBase* const Task = GetOwningTask();
	if (!IsValid(Task))
	{
		return false;
	}

	return UAzSpeechSettings::CheckAzSpeechSettings(Task->TaskOptions, Task->bIsSSMLBased) && UAzSpeechTaskStatus::IsTaskStillValid(Task);
}

std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig> FAzSpeechRunnableBase::CreateSpeechConfig() const
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Creating Azure SDK speech config"), *GetThreadName(), *FString(__func__));

	if (!UAzSpeechTaskStatus::IsTaskStillValid(GetOwningTask()))
	{
		return nullptr;
	}

	if (OwningTask->GetTaskOptions().bUsePrivateEndpoint)
	{
		return Microsoft::CognitiveServices::Speech::SpeechConfig::FromEndpoint(TCHAR_TO_UTF8(*OwningTask->GetTaskOptions().PrivateEndpoint.ToString()), TCHAR_TO_UTF8(*OwningTask->GetTaskOptions().SubscriptionKey.ToString()));
	}

	return Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(TCHAR_TO_UTF8(*OwningTask->GetTaskOptions().SubscriptionKey.ToString()), TCHAR_TO_UTF8(*OwningTask->GetTaskOptions().RegionID.ToString()));
}

const std::chrono::seconds FAzSpeechRunnableBase::GetTaskTimeout() const
{
	return std::chrono::seconds(GetTimeout());
}

const bool FAzSpeechRunnableBase::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const
{
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
		const FString LogFilename = "AzSpeech " + FDateTime::Now().ToString() + ".log";
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
	if (UAzSpeechTaskStatus::IsTaskStillValid(GetOwningTask()))
	{
		switch (OwningTask->GetTaskOptions().ProfanityFilter)
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
	UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Error details: %s"), *GetThreadName(), *FString(__func__), UTF8_TO_TCHAR(ErrorDetails.c_str()));
	UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Log generated in directory: %s"), *GetThreadName(), *FString(__func__), *UAzSpeechHelper::GetAzSpeechLogsBaseDir());
}

const EThreadPriority FAzSpeechRunnableBase::GetCPUThreadPriority() const
{
	if (UAzSpeechTaskStatus::IsTaskStillValid(GetOwningTask()))
	{
		switch (UAzSpeechSettings::Get()->TasksThreadPriority)
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
	if (UAzSpeechTaskStatus::IsTaskStillValid(GetOwningTask()))
	{
		return UAzSpeechSettings::Get()->ThreadUpdateInterval <= 0.f ? 0.1f : UAzSpeechSettings::Get()->ThreadUpdateInterval;
	}

	return 0.1f;
}

const int32 FAzSpeechRunnableBase::GetTimeout() const
{
	if (UAzSpeechTaskStatus::IsTaskStillValid(GetOwningTask()))
	{
		return UAzSpeechSettings::Get()->TimeOutInSeconds <= 0.f ? 15.f : UAzSpeechSettings::Get()->TimeOutInSeconds;
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
