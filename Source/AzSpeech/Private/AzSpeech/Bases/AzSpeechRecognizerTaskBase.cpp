// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Bases/AzSpeechRecognizerTaskBase.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include "Async/Async.h"

void UAzSpeechRecognizerTaskBase::Activate()
{
	Super::Activate();
}

void UAzSpeechRecognizerTaskBase::StopAzSpeechTask()
{
	Super::StopAzSpeechTask();

	if (!RecognizerObject)
	{
		return;
	}
	
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
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech Task: %s (%s): %s; Trying to enable continuos recognition with invalid recognizer"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

		StopAzSpeechTask();
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Enabling continuous recognition"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
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
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech Task: %s (%s): %s; Trying to disable continuos recognition with invalid recognizer"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

		StopAzSpeechTask();
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Disabling continuous recognition"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	if (!RecognizerObject->IsEnabled())
	{
		return;
	}

	RecognizerObject->Disable();
}

const FString UAzSpeechRecognizerTaskBase::GetLastRecognizedString() const
{
	if (LastRecognizedString.empty())
	{
		return FString();
	}

	return UTF8_TO_TCHAR(LastRecognizedString.c_str());
}

bool UAzSpeechRecognizerTaskBase::StartAzureTaskWork_Internal()
{
	return Super::StartAzureTaskWork_Internal();
}

void UAzSpeechRecognizerTaskBase::ClearBindings()
{
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

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Adding extra settings to existing recognizer object"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

	const auto RecognitionUpdate_Lambda = [this](const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
	{
		OnRecognitionUpdated(RecognitionEventArgs);
	};

	RecognizerObject->Recognized.Connect(RecognitionUpdate_Lambda);

	if (bContinuousRecognition)
	{
		RecognizerObject->Recognizing.Connect(RecognitionUpdate_Lambda);
	}
}

void UAzSpeechRecognizerTaskBase::BroadcastFinalResult()
{	
	AsyncTask(ENamedThreads::GameThread, [=] 
	{ 
		RecognitionCompleted.Broadcast(GetLastRecognizedString());
		Super::BroadcastFinalResult();
	});
}

void UAzSpeechRecognizerTaskBase::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig)
{
	Super::ApplySDKSettings(InConfig);

	if (IsUsingAutoLanguage())
	{
		return;
	}
	
	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Using language: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *LanguageId);

	const std::string UsedLang = TCHAR_TO_UTF8(*LanguageId);
	InConfig->SetSpeechRecognitionLanguage(UsedLang);
}

void UAzSpeechRecognizerTaskBase::OnRecognitionUpdated(const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
{
	if (!ProcessRecognitionResult(RecognitionEventArgs.Result))
	{
		return;
	}

	LastRecognizedString.clear();
	LastRecognizedString = RecognitionEventArgs.Result->Text;
	LastRecognizedString.shrink_to_fit();

	if (AzSpeech::Internal::GetPluginSettings()->bEnableRuntimeDebug)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Current recognized text: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *GetLastRecognizedString());
		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Current duration: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(RecognitionEventArgs.Result->Duration()));
		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Current offset: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(RecognitionEventArgs.Result->Offset()));
		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Current reason code: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(static_cast<int32>(RecognitionEventArgs.Result->Reason)));
		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Current result id: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString(UTF8_TO_TCHAR(RecognitionEventArgs.Result->ResultId.c_str())));
	}

	switch (RecognitionEventArgs.Result->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Task completed with result: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *GetLastRecognizedString());
			BroadcastFinalResult();

			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizingSpeech:
			AsyncTask(ENamedThreads::GameThread, [=] { RecognitionUpdated.Broadcast(GetLastRecognizedString()); });
			break;
	}
}

bool UAzSpeechRecognizerTaskBase::InitializeRecognizer(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig)
{
	if (!AzSpeech::Internal::CheckAzSpeechSettings())
	{
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Initializing recognizer object"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

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
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech Task: %s (%s): %s; Task failed. Result: Invalid candidate languages"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return false;
		}

		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Initializing auto language detection"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
		for (const std::string& Iterator : Candidates)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Using language candidate: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString(UTF8_TO_TCHAR(Iterator.c_str())));
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

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Starting recognition"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

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

const bool UAzSpeechRecognizerTaskBase::ProcessRecognitionResult(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognitionResult>& Result) const
{
	switch (Result->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Task completed. Reason: RecognizedSpeech"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizingSpeech:
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech Task: %s (%s): %s; Task running. Reason: RecognizingSpeech"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::NoMatch:
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech Task: %s (%s): %s; Task failed. Reason: NoMatch"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return false;

		default:
			break;
	}

	if (Result->Reason == Microsoft::CognitiveServices::Speech::ResultReason::Canceled)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech Task: %s (%s): %s; Task failed. Reason: Canceled"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::CancellationDetails::FromResult(Result);

		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech Task: %s (%s): %s; Cancellation Reason: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

		if (CancellationDetails->Reason == Microsoft::CognitiveServices::Speech::CancellationReason::Error)
		{
			ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
		}

		return false;
	}

	UE_LOG(LogAzSpeech, Warning, TEXT("AzSpeech Task: %s (%s): %s; Ended with undefined reason"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	return false;
}
