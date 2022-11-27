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

void UAzSpeechRecognizerTaskBase::StopAzureTaskWork()
{
	Super::StopAzureTaskWork();

	if (!IsTaskStillValid(this) || !RecognizerObject)
	{
		SetReadyToDestroy();
		return;
	}

	if (bContinuousRecognition)
	{
		if (bCanBroadcastFinal)
		{
			BroadcastFinalResult();
		}

		AsyncTask(AzSpeech::Internal::GetBackgroundThread(), [this]
		{
			if (!IsTaskStillValid(this))
			{
				return;
			}

			FScopeLock Lock(&Mutex);
			
			if (RecognizerObject)
			{
				RecognizerObject->StopContinuousRecognitionAsync().wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
			}

			SetReadyToDestroy();
		});
	}
}

void UAzSpeechRecognizerTaskBase::EnableContinuousRecognition()
{
	if (!IsTaskStillValid(this) || !RecognizerObject)
	{
		SetReadyToDestroy();
		return;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%s); Function: %s; Message: Enabling continuous recognition"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	if (RecognizerObject->IsEnabled())
	{
		return;
	}

	RecognizerObject->Enable();
}

void UAzSpeechRecognizerTaskBase::DisableContinuousRecognition()
{
	if (!IsTaskStillValid(this) || !RecognizerObject)
	{
		SetReadyToDestroy();
		return;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%s); Function: %s; Message: Disabling continuous recognition"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	if (!RecognizerObject->IsEnabled())
	{
		return;
	}

	RecognizerObject->Disable();
}

const FString UAzSpeechRecognizerTaskBase::GetLastRecognizedString() const
{
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

void UAzSpeechRecognizerTaskBase::ClearAllBindings()
{
	Super::ClearAllBindings();

	if (RecognitionCompleted.IsBound())
	{
		RecognitionCompleted.RemoveAll(this);
	}

	if (RecognitionUpdated.IsBound())
	{
		RecognitionUpdated.RemoveAll(this);
	}

	DisconnectRecognitionSignals();
}

void UAzSpeechRecognizerTaskBase::ConnectTaskSignals()
{
	Super::ConnectTaskSignals();

	if (!IsTaskStillValid(this) || !RecognizerObject)
	{
		SetReadyToDestroy();
		return;
	}

	const auto RecognitionUpdate_Lambda = [this](const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
	{
		FScopeLock Lock(&Mutex);

		if (!IsTaskStillValid(this))
		{
			SetReadyToDestroy();
			return;
		}

		LastRecognitionResult = RecognitionEventArgs.Result;
		AsyncTask(ENamedThreads::GameThread, [this] { OnRecognitionUpdated(); });
	};

	RecognizerObject->Recognized.Connect(RecognitionUpdate_Lambda);

	if (bContinuousRecognition)
	{
		RecognizerObject->Recognizing.Connect(RecognitionUpdate_Lambda);
	}
}

void UAzSpeechRecognizerTaskBase::ReleaseResources()
{
	Super::ReleaseResources();

	if (!IsValid(this))
	{
		return;
	}

	RecognizerObject = nullptr;
}

void UAzSpeechRecognizerTaskBase::BroadcastFinalResult()
{
	Super::BroadcastFinalResult();
	
	if (RecognitionCompleted.IsBound())
	{
		RecognitionCompleted.Broadcast(GetLastRecognizedString());
	}
	
	SetReadyToDestroy();
}

void UAzSpeechRecognizerTaskBase::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig)
{
	Super::ApplySDKSettings(InConfig);

	if (IsUsingAutoLanguage())
	{
		return;
	}
	
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%s); Function: %s; Message: Using language: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *LanguageId);

	const std::string UsedLang = TCHAR_TO_UTF8(*LanguageId);
	InConfig->SetSpeechRecognitionLanguage(UsedLang);
}

void UAzSpeechRecognizerTaskBase::OnRecognitionUpdated()
{
	check(IsInGameThread());
	FScopeLock Lock(&Mutex);
	
	if (!IsTaskStillValid(this) || !LastRecognitionResult)
	{
		SetReadyToDestroy();
		return;
	}

	if (!bRecognizingStatusAlreadyShown)
	{
		if (RecognitionStarted.IsBound())
		{
			RecognitionStarted.Broadcast();
		}
	}

	if (!ProcessRecognitionResult())
	{
		SetReadyToDestroy();
		return;
	}

	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%s); Function: %s; Message: Current recognized text: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *GetLastRecognizedString());
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%s); Function: %s; Message: Current duration: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(LastRecognitionResult->Duration()));
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%s); Function: %s; Message: Current offset: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(LastRecognitionResult->Offset()));
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%s); Function: %s; Message: Current reason code: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(static_cast<int32>(LastRecognitionResult->Reason)));
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%s); Function: %s; Message: Current result id: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString(UTF8_TO_TCHAR(LastRecognitionResult->ResultId.c_str())));

	switch (LastRecognitionResult->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%s); Function: %s; Message: Task completed with result: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *GetLastRecognizedString());
			BroadcastFinalResult();

			// Also broadcast the final result on update delegate
			if (RecognitionUpdated.IsBound())
			{
				RecognitionUpdated.Broadcast(GetLastRecognizedString());
			}

			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizingSpeech:
			if (RecognitionUpdated.IsBound())
			{
				RecognitionUpdated.Broadcast(GetLastRecognizedString());
			}
			break;
	}
}

void UAzSpeechRecognizerTaskBase::StartRecognitionWork(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig)
{
	AsyncTask(AzSpeech::Internal::GetBackgroundThread(), [this, InAudioConfig, FuncName = __func__]
	{		
		if (!IsTaskStillValid(this))
		{
			return;
		}

		FScopeLock Lock(&Mutex);

		if (!InitializeRecognizer(InAudioConfig))
		{
			SetReadyToDestroy();
			return;
		}

		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Starting recognition"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(FuncName));

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

bool UAzSpeechRecognizerTaskBase::InitializeRecognizer(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig)
{
	if (!IsTaskStillValid(this))
	{
		return false;
	}

	if (!AzSpeech::Internal::CheckAzSpeechSettings())
	{
		SetReadyToDestroy();
		return false;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%s); Function: %s; Message: Initializing recognizer object"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

	const auto SpeechConfig = UAzSpeechTaskBase::CreateSpeechConfig();

	if (!SpeechConfig)
	{
		SetReadyToDestroy();
		return false;
	}

	ApplySDKSettings(SpeechConfig);

	if (IsUsingAutoLanguage())
	{
		const std::vector<std::string> Candidates = AzSpeech::Internal::GetCandidateLanguages(TaskName, GetUniqueID(), bContinuousRecognition);
		if (Candidates.empty())
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%s); Function: %s; Message: Task failed. Result: Invalid candidate languages"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

			SetReadyToDestroy();
			return false;
		}

		RecognizerObject = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(SpeechConfig, Microsoft::CognitiveServices::Speech::AutoDetectSourceLanguageConfig::FromLanguages(Candidates), InAudioConfig);
	}
	else
	{
		RecognizerObject = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	ConnectTaskSignals();

	return true;
}

const bool UAzSpeechRecognizerTaskBase::ProcessRecognitionResult()
{
	if (!LastRecognitionResult)
	{
		SetReadyToDestroy();
		return false;
	}

	switch (LastRecognitionResult->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%s); Function: %s; Message: Task completed. Reason: RecognizedSpeech"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizingSpeech:
			if (!bRecognizingStatusAlreadyShown)
			{
				UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%s); Function: %s; Message: Task started. Reason: RecognizingSpeech"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
				bRecognizingStatusAlreadyShown = true;
			}
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%s); Function: %s; Message: Task running. Reason: RecognizingSpeech"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::NoMatch:
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%s); Function: %s; Message: Task failed. Reason: NoMatch"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return false;

		default:
			break;
	}

	if (LastRecognitionResult->Reason == Microsoft::CognitiveServices::Speech::ResultReason::Canceled)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%s); Function: %s; Message: Task failed. Reason: Canceled"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::CancellationDetails::FromResult(LastRecognitionResult);

		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%s); Function: %s; Message: Cancellation Reason: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

		if (CancellationDetails->Reason == Microsoft::CognitiveServices::Speech::CancellationReason::Error)
		{
			ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
		}

		return false;
	}

	UE_LOG(LogAzSpeech_Internal, Warning, TEXT("Task: %s (%s); Function: %s; Message: Ended with undefined reason"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	return false;
}

void UAzSpeechRecognizerTaskBase::DisconnectRecognitionSignals()
{
	if (!RecognizerObject)
	{
		return;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%s); Function: %s; Message: Disconnecting Azure SDK recognition signals"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

	SignalDisconecter_T(RecognizerObject->Recognizing);
	SignalDisconecter_T(RecognizerObject->Recognized);
}
