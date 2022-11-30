// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"
#include "AzSpeech/Tasks/Bases/AzSpeechTaskBase.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include <HAL/PlatformFileManager.h>
#include <Misc/FileHelper.h>
#include <Async/Async.h>

FAzSpeechRunnableBase::FAzSpeechRunnableBase(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig) : OwningTask(InOwningTask), AudioConfig(InAudioConfig)
{
}

void FAzSpeechRunnableBase::StartAzSpeechRunnableTask()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Creating new runnable thread to initialize work"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
	Thread.Reset(FRunnableThread::Create(this, *FString::Printf(TEXT("AzSpeech_%s_%d"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID()), 0u, AzSpeech::Internal::GetCPUThreadPriority()));
}

void FAzSpeechRunnableBase::StopAzSpeechRunnableTask()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Setting runnable work as pending stop"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
	bStopTask = true;
}

bool FAzSpeechRunnableBase::IsPendingStop() const
{
	return bStopTask;
}

bool FAzSpeechRunnableBase::Init()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Initializing runnable thread"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
	
	return CanInitializeTask();
}

uint32 FAzSpeechRunnableBase::Run()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Running runnable thread work"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
	
	return InitializeAzureObject() ? 1u : 0u;
}

void FAzSpeechRunnableBase::Stop()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Stopping runnable thread work"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
}

void FAzSpeechRunnableBase::Exit()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Exiting thread"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

	if (UAzSpeechTaskBase::IsTaskStillValid(OwningTask) && OwningTask->IsTaskActive())
	{
		AsyncTask(ENamedThreads::GameThread, [this] { OwningTask->BroadcastFinalResult(); });
	}

	ClearSignals();
	RemoveBindings();

	if (!OwningTask->IsTaskReadyToDestroy())
	{
		OwningTask->SetReadyToDestroy();
	}
}

bool FAzSpeechRunnableBase::InitializeAzureObject()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Initializing Azure Object"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
	
	return true;
}

bool FAzSpeechRunnableBase::CanInitializeTask()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Checking if can initialize task in current context"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
	
	bool bOutput = true;

	if (!AzSpeech::Internal::CheckAzSpeechSettings())
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Failed to initialize task due to invalid settings"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

		bOutput = false;
	}

	if (!IsValid(OwningTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Failed to initialize task due to invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		
		bOutput = false;
	}

	return bOutput;
}

void FAzSpeechRunnableBase::ClearSignals()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Disconnecting Azure SDK recognition signals"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
}

void FAzSpeechRunnableBase::RemoveBindings()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Removing existing delegate bindings"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
}

std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig> FAzSpeechRunnableBase::CreateSpeechConfig() const
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Creating Azure SDK speech config"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

	const auto Settings = AzSpeech::Internal::GetAzSpeechKeys();
	const auto SpeechConfig = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));

	if (!SpeechConfig)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Failed to create speech configuration"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

		return nullptr;
	}

	return SpeechConfig;
}

const std::chrono::seconds FAzSpeechRunnableBase::GetTaskTimeout() const
{
	return std::chrono::seconds(AzSpeech::Internal::GetTimeout());
}


bool FAzSpeechRunnableBase::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const
{
	if (!InSpeechConfig)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid speech config"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Applying Azure SDK Settings"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

	EnableLogInConfiguration(InSpeechConfig);

	InSpeechConfig->SetProfanity(AzSpeech::Internal::GetProfanityFilter());

	if (OwningTask->IsUsingAutoLanguage())
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Using auto language identification"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		
		InSpeechConfig->SetProperty(Microsoft::CognitiveServices::Speech::PropertyId::SpeechServiceConnection_SingleLanguageIdPriority, "Latency");
	}

	return true;
}

bool FAzSpeechRunnableBase::EnableLogInConfiguration(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig) const
{
	if (!InSpeechConfig)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid speech config"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	if (!AzSpeech::Internal::GetPluginSettings()->bEnableSDKLogs)
	{
		return true;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Enabling Azure SDK log"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

	if (FString AzSpeechLogPath = AzSpeech::Internal::GetAzSpeechLogsBaseDir();
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*AzSpeechLogPath))
	{
		AzSpeechLogPath += "/UEAzSpeech " + FDateTime::Now().ToString() + ".log";

		if (FFileHelper::SaveStringToFile(FString(), *AzSpeechLogPath))
		{
			InSpeechConfig->SetProperty(Microsoft::CognitiveServices::Speech::PropertyId::Speech_LogFilename, TCHAR_TO_UTF8(*AzSpeechLogPath));

			return true;
		}
	}

	return false;
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

	UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Error code: %s"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), *ErrorCodeStr);
	
	UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Error Details: %s"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), *UTF8_TO_TCHAR(ErrorDetails.c_str()));
	
	UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Log generated in directory: %s"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), *AzSpeech::Internal::GetAzSpeechLogsBaseDir());
}