// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechRecognizerTaskBase.h"
#include "AzSpeechInternalFuncs.h"
#include "Async/Async.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx.h>
THIRD_PARTY_INCLUDES_END

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
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Trying to enable continuos recognition with invalid recognizer"), *FString(__func__));

		StopAzSpeechTask();
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: Enabling continuous recognition"), *FString(__func__));
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
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Trying to disable continuos recognition with invalid recognizer"), *FString(__func__));

		StopAzSpeechTask();
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: Disabling continuous recognition"), *FString(__func__));
	if (!RecognizerObject->IsEnabled())
	{
		return;
	}

	RecognizerObject->Disable();
}

const FString UAzSpeechRecognizerTaskBase::GetLastRecognizedString() const
{
	return UTF8_TO_TCHAR(LastRecognizedString.c_str());
}

bool UAzSpeechRecognizerTaskBase::StartAzureTaskWork_Internal()
{
	return Super::StartAzureTaskWork_Internal();
}

void UAzSpeechRecognizerTaskBase::ClearBindings()
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s: Removing existing bindings."), *FString(__func__));

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

	UE_LOG(LogAzSpeech, Display, TEXT("%s: Adding extra settings to existing recognizer object."), *FString(__func__));

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

void UAzSpeechRecognizerTaskBase::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig)
{
	Super::ApplySDKSettings(InConfig);

	if (IsUsingAutoLanguage())
	{
		return;
	}
	
	UE_LOG(LogAzSpeech, Display, TEXT("%s: Using language: %s"), *FString(__func__), *LanguageId);

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
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Current recognized text: %s"), *FString(__func__), *GetLastRecognizedString());
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Current duration: %s"), *FString(__func__), *FString::FromInt(RecognitionEventArgs.Result->Duration()));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Current offset: %s"), *FString(__func__), *FString::FromInt(RecognitionEventArgs.Result->Offset()));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Current reason code: %s"), *FString(__func__), *FString::FromInt(static_cast<int32>(RecognitionEventArgs.Result->Reason)));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Current result id: %s"), *FString(__func__), *FString(UTF8_TO_TCHAR(RecognitionEventArgs.Result->ResultId.c_str())));
	}

	switch (RecognitionEventArgs.Result->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech, Display, TEXT("%s: Task finished with result: %s"), *FString(__func__), *GetLastRecognizedString());
			RecognitionCompleted.Broadcast(GetLastRecognizedString());
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

	UE_LOG(LogAzSpeech, Display, TEXT("%s: Initializing recognizer object."), *FString(__func__));

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
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Task failed. Result: Invalid candidate languages"), *FString(__func__));
			return false;
		}

		UE_LOG(LogAzSpeech, Display, TEXT("%s: Initializing language auto detection"), *FString(__func__));
		for (const std::string& Iterator : Candidates)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("%s: Candidate: %s"), *FString(__func__), *FString(UTF8_TO_TCHAR(Iterator.c_str())));
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

	UE_LOG(LogAzSpeech, Display, TEXT("%s: Starting recognition."), *FString(__func__));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
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
			UE_LOG(LogAzSpeech, Display, TEXT("%s: Task completed. Reason: RecognizedSpeech"), *FString(__func__));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizingSpeech:
			UE_LOG(LogAzSpeech, Display, TEXT("%s: Task running. Reason: RecognizingSpeech"), *FString(__func__));
			return true;

		default:
			break;
	}

	if (Result->Reason == Microsoft::CognitiveServices::Speech::ResultReason::Canceled)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Task failed. Reason: Canceled"), *FString(__func__));
		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::CancellationDetails::FromResult(Result);

		UE_LOG(LogAzSpeech, Error, TEXT("%s: Cancellation Reason: %s"), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

		if (CancellationDetails->Reason == Microsoft::CognitiveServices::Speech::CancellationReason::Error)
		{
			ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
		}

		return false;
	}

	UE_LOG(LogAzSpeech, Warning, TEXT("%s: Undefined reason"), *FString(__func__));
	return false;
}
