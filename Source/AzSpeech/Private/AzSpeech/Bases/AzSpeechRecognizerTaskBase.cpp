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
		UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Trying to enable continuos recognition with invalid recognizer"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

		StopAzSpeechTask();
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Enabling continuous recognition"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
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
		UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Trying to disable continuos recognition with invalid recognizer"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

		StopAzSpeechTask();
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Disabling continuous recognition"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
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

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Adding extra settings to existing recognizer object"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

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
	Super::BroadcastFinalResult();

	AsyncTask(ENamedThreads::GameThread, [=] { RecognitionCompleted.Broadcast(GetLastRecognizedString()); });
}

void UAzSpeechRecognizerTaskBase::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig)
{
	Super::ApplySDKSettings(InConfig);

	if (IsUsingAutoLanguage())
	{
		return;
	}
	
	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Using language: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *LanguageId);

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
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current recognized text: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *GetLastRecognizedString());
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current duration: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString::FromInt(RecognitionEventArgs.Result->Duration()));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current offset: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString::FromInt(RecognitionEventArgs.Result->Offset()));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current reason code: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString::FromInt(static_cast<int32>(RecognitionEventArgs.Result->Reason)));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current result id: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(UTF8_TO_TCHAR(RecognitionEventArgs.Result->ResultId.c_str())));
	}

	switch (RecognitionEventArgs.Result->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Task completed with result: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *GetLastRecognizedString());
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

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Initializing recognizer object"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

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
			UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Task failed. Result: Invalid candidate languages"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
			return false;
		}

		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Initializing auto language detection"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
		for (const std::string& Iterator : Candidates)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Using language candidate: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(UTF8_TO_TCHAR(Iterator.c_str())));
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

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Starting recognition"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

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
			UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Task completed. Reason: RecognizedSpeech"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizingSpeech:
			UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Task running. Reason: RecognizingSpeech"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::NoMatch:
			UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Task failed. Reason: NoMatch"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
			return false;

		default:
			break;
	}

	if (Result->Reason == Microsoft::CognitiveServices::Speech::ResultReason::Canceled)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Task failed. Reason: Canceled"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::CancellationDetails::FromResult(Result);

		UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Cancellation Reason: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *CancellationReasonToString(CancellationDetails->Reason));

		if (CancellationDetails->Reason == Microsoft::CognitiveServices::Speech::CancellationReason::Error)
		{
			ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
		}

		return false;
	}

	UE_LOG(LogAzSpeech, Warning, TEXT("%s: AzSpeech Task: %s (%s); Ended with undefined reason"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
	return false;
}
