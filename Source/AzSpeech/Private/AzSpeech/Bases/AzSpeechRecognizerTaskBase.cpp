// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Bases/AzSpeechRecognizerTaskBase.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include "Async/Async.h"

void UAzSpeechRecognizerTaskBase::Activate()
{
	bRecognizingStatusAlreadyShown = false;
	Super::Activate();
}

void UAzSpeechRecognizerTaskBase::StopAzSpeechTask()
{
	Super::StopAzSpeechTask();

	if (!RecognizerObject)
	{
		return;
	}

	FScopeLock Lock(&Mutex);
	
	if (bContinuousRecognition)
	{
		if (!bAlreadyBroadcastFinal)
		{
			BroadcastFinalResult();
		}

		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]
		{
			if (!Mutex.TryLock())
			{
				return;
			}

			RecognizerObject->StopContinuousRecognitionAsync().wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
		});
	}
}

void UAzSpeechRecognizerTaskBase::EnableContinuousRecognition()
{
	if (!RecognizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%s); Function: %s; Message: Trying to enable continuos recognition with invalid recognizer"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

		StopAzSpeechTask();
		return;
	}

	FScopeLock Lock(&Mutex);

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Enabling continuous recognition"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	if (RecognizerObject->IsEnabled())
	{
		return;
	}

	RecognizerObject->Enable();
}

void UAzSpeechRecognizerTaskBase::DisableContinuousRecognition()
{
	if (!RecognizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%s); Function: %s; Message: Trying to disable continuos recognition with invalid recognizer"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

		StopAzSpeechTask();
		return;
	}

	FScopeLock Lock(&Mutex);

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Disabling continuous recognition"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	if (!RecognizerObject->IsEnabled())
	{
		return;
	}

	RecognizerObject->Disable();
}

const FString UAzSpeechRecognizerTaskBase::GetLastRecognizedString() const
{
	FScopeLock Lock(&Mutex);
	
	if (!LastRecognitionResult)
	{
		return FString();
	}

	const std::string LastRecognizedString = LastRecognitionResult->Text;
	
	if (LastRecognizedString.empty())
	{
		return FString();
	}

	return UTF8_TO_TCHAR(LastRecognizedString.c_str());
}

void UAzSpeechRecognizerTaskBase::ClearBindings()
{
	Super::ClearBindings();

	if (RecognitionCompleted.IsBound())
	{
		RecognitionCompleted.RemoveAll(this);
	}

	if (RecognitionUpdated.IsBound())
	{
		RecognitionUpdated.RemoveAll(this);
	}

	if (!RecognizerObject)
	{
		return;
	}

	FScopeLock Lock(&Mutex);

	SignalDisconecter_T(RecognizerObject->Recognizing);
	SignalDisconecter_T(RecognizerObject->Recognized);
}

void UAzSpeechRecognizerTaskBase::ApplyExtraSettings()
{
	Super::ApplyExtraSettings();

	if (!RecognizerObject)
	{
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Adding extra settings to existing recognizer object"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

	const auto RecognitionUpdate_Lambda = [this](const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
	{
		LastRecognitionResult = RecognitionEventArgs.Result;
		AsyncTask(ENamedThreads::GameThread, [this] { OnRecognitionUpdated(); });
	};

	FScopeLock Lock(&Mutex);

	RecognizerObject->Recognized.Connect(RecognitionUpdate_Lambda);

	if (bContinuousRecognition)
	{
		RecognizerObject->Recognizing.Connect(RecognitionUpdate_Lambda);
	}
}

void UAzSpeechRecognizerTaskBase::BroadcastFinalResult()
{
	check(IsInGameThread());
	
	FScopeLock Lock(&Mutex);
	
	RecognitionCompleted.Broadcast(GetLastRecognizedString());
	Super::BroadcastFinalResult();
}

void UAzSpeechRecognizerTaskBase::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig)
{
	Super::ApplySDKSettings(InConfig);

	if (IsUsingAutoLanguage())
	{
		return;
	}
	
	FScopeLock Lock(&Mutex);
	
	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Using language: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *LanguageId);

	const std::string UsedLang = TCHAR_TO_UTF8(*LanguageId);
	InConfig->SetSpeechRecognitionLanguage(UsedLang);
}

void UAzSpeechRecognizerTaskBase::OnRecognitionUpdated()
{
	check(IsInGameThread());

	FScopeLock Lock(&Mutex);

	if (!LastRecognitionResult)
	{
		return;
	}

	if (!ProcessRecognitionResult())
	{
		ClearBindings();
		return;
	}

	if (AzSpeech::Internal::GetPluginSettings()->bEnableRuntimeDebug)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current recognized text: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *GetLastRecognizedString());
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current duration: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(LastRecognitionResult->Duration()));
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current offset: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(LastRecognitionResult->Offset()));
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current reason code: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(static_cast<int32>(LastRecognitionResult->Reason)));
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current result id: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString(UTF8_TO_TCHAR(LastRecognitionResult->ResultId.c_str())));
	}

	switch (LastRecognitionResult->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Task completed with result: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *GetLastRecognizedString());
			BroadcastFinalResult();

			// Also broadcast the final result on update delegate
			RecognitionUpdated.Broadcast(GetLastRecognizedString());

			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizingSpeech:
			RecognitionUpdated.Broadcast(GetLastRecognizedString());
			break;
	}
}

bool UAzSpeechRecognizerTaskBase::InitializeRecognizer(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig)
{
	if (!AzSpeech::Internal::CheckAzSpeechSettings())
	{
		return false;
	}

	FScopeLock Lock(&Mutex);

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Initializing recognizer object"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

	const auto SpeechConfig = UAzSpeechTaskBase::CreateSpeechConfig();

	if (!SpeechConfig)
	{
		return false;
	}

	ApplySDKSettings(SpeechConfig);

	if (IsUsingAutoLanguage())
	{
		const std::vector<std::string> Candidates = AzSpeech::Internal::GetCandidateLanguages();
		if (Candidates.empty())
		{
			UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%s); Function: %s; Message: Task failed. Result: Invalid candidate languages"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return false;
		}

		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Initializing auto language detection"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
		for (const std::string& Iterator : Candidates)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Using language candidate: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString(UTF8_TO_TCHAR(Iterator.c_str())));
		}

		RecognizerObject = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(SpeechConfig, Microsoft::CognitiveServices::Speech::AutoDetectSourceLanguageConfig::FromLanguages(Candidates), InAudioConfig);
	}
	else
	{
		RecognizerObject = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	ApplyExtraSettings();

	return true;
}

void UAzSpeechRecognizerTaskBase::StartRecognitionWork()
{
	if (!RecognizerObject)
	{
		return;
	}
	
	FScopeLock Lock(&Mutex);

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Starting recognition"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]
	{
		if (bContinuousRecognition)
		{
			RecognizerObject->StartContinuousRecognitionAsync().wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
		}
		else
		{
			RecognizerObject->RecognizeOnceAsync().wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
		}
	});
}

const bool UAzSpeechRecognizerTaskBase::ProcessRecognitionResult()
{
	FScopeLock Lock(&Mutex);
	
	switch (LastRecognitionResult->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Task completed. Reason: RecognizedSpeech"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizingSpeech:
			if (!bRecognizingStatusAlreadyShown)
			{
				UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Task running. Reason: RecognizingSpeech"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
				bRecognizingStatusAlreadyShown = true;
			}
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::NoMatch:
			UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%s); Function: %s; Message: Task failed. Reason: NoMatch"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return false;

		default:
			break;
	}

	if (LastRecognitionResult->Reason == Microsoft::CognitiveServices::Speech::ResultReason::Canceled)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%s); Function: %s; Message: Task failed. Reason: Canceled"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::CancellationDetails::FromResult(LastRecognitionResult);

		UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%s); Function: %s; Message: Cancellation Reason: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

		if (CancellationDetails->Reason == Microsoft::CognitiveServices::Speech::CancellationReason::Error)
		{
			ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
		}

		return false;
	}

	UE_LOG(LogAzSpeech, Warning, TEXT("Task: %s (%s); Function: %s; Message: Ended with undefined reason"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	return false;
}
