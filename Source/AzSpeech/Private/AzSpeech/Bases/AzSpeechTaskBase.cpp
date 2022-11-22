// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Bases/AzSpeechTaskBase.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include "Misc/FileHelper.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

void UAzSpeechTaskBase::Activate()
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Activating"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

	AzSpeech::Internal::GetLanguageID(LanguageId);

	Super::Activate();
	
	StartAzureTaskWork_Internal();

#if WITH_EDITOR
	FEditorDelegates::PrePIEEnded.AddUObject(this, &UAzSpeechTaskBase::PrePIEEnded);
#endif
}

void UAzSpeechTaskBase::StopAzSpeechTask()
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Finishing task"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
	
	bHasStopped = true;
	ClearBindings();
}

const bool UAzSpeechTaskBase::IsTaskStillValid(const UAzSpeechTaskBase* Test)
{
	return IsValid(Test) && !Test->bIsReadyToDestroy && !Test->bHasStopped;
}

bool UAzSpeechTaskBase::StartAzureTaskWork_Internal()
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Starting Azure SDK task"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

	return AzSpeech::Internal::CheckAzSpeechSettings();
}

void UAzSpeechTaskBase::SetReadyToDestroy()
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Setting task as Ready to Destroy"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

	bIsReadyToDestroy = true;
	Super::SetReadyToDestroy();
}

void UAzSpeechTaskBase::ClearBindings()
{
	if (!bAlreadyUnbound)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Removing existing bindings"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
	}

	bAlreadyUnbound = true;
}

void UAzSpeechTaskBase::BroadcastFinalResult()
{
	bAlreadyBroadcastFinal = true;
}

const bool UAzSpeechTaskBase::IsUsingAutoLanguage() const
{
	return LanguageId.Equals("Auto", ESearchCase::IgnoreCase);
}

#if WITH_EDITOR
void UAzSpeechTaskBase::PrePIEEnded(bool bIsSimulating)
{	
	FEditorDelegates::PrePIEEnded.RemoveAll(this);

	if (UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Trying to finish task due to PIE end"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
		StopAzSpeechTask();
	}
}
#endif

void UAzSpeechTaskBase::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig)
{
	if (!InSpeechConfig)
	{
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Applying Azure SDK Settings"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

	EnableLogInConfiguration(InSpeechConfig);

	InSpeechConfig->SetProfanity(AzSpeech::Internal::GetProfanityFilter());

	if (IsUsingAutoLanguage())
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Using auto language identification"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
		InSpeechConfig->SetProperty(Microsoft::CognitiveServices::Speech::PropertyId::SpeechServiceConnection_SingleLanguageIdPriority, "Latency");
	}
}

void UAzSpeechTaskBase::EnableLogInConfiguration(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig)
{
	if (!InSpeechConfig)
	{
		return;
	}

	if (!AzSpeech::Internal::GetPluginSettings()->bEnableSDKLogs)
	{
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Enabling Azure SDK log"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

	if (FString AzSpeechLogPath = AzSpeech::Internal::GetAzSpeechLogsBaseDir();
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*AzSpeechLogPath))
	{
		AzSpeechLogPath += "/UEAzSpeech " + FDateTime::Now().ToString() + ".log";

		if (FFileHelper::SaveStringToFile(FString(), *AzSpeechLogPath))
		{
			InSpeechConfig->SetProperty(Microsoft::CognitiveServices::Speech::PropertyId::Speech_LogFilename, TCHAR_TO_UTF8(*AzSpeechLogPath));
		}
	}
}

const FString UAzSpeechTaskBase::CancellationReasonToString(const Microsoft::CognitiveServices::Speech::CancellationReason& CancellationReason) const
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

void UAzSpeechTaskBase::ProcessCancellationError(const Microsoft::CognitiveServices::Speech::CancellationErrorCode& ErrorCode, const std::string& ErrorDetails) const
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

	UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Error code: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *ErrorCodeStr);

	const FString ErrorDetailsStr = UTF8_TO_TCHAR(ErrorDetails.c_str());
	UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Error Details: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *ErrorDetailsStr);
	UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Log generated in directory: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *AzSpeech::Internal::GetAzSpeechLogsBaseDir());
}

std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig> UAzSpeechTaskBase::CreateSpeechConfig()
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Creating Azure SDK speech config"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
	
	const auto Settings = AzSpeech::Internal::GetAzSpeechKeys();
	const auto SpeechConfig = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));

	if (!SpeechConfig)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Failed to create speech configuration"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
		return nullptr;
	}

	return SpeechConfig;
}
