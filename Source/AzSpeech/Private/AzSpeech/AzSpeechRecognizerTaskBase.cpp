// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechRecognizerTaskBase.h"
#include "AzSpeechInternalFuncs.h"
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
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]
		{
			if (!Mutex.TryLock())
			{
				return;
			}

			RecognizerObject->StopContinuousRecognitionAsync().wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
			RecognizerObject.reset();
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
	if (!RecognizerObject)
	{
		return;
	}

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

void UAzSpeechRecognizerTaskBase::OnRecognitionUpdated(const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
{
	if (!AzSpeech::Internal::ProcessRecognitionResult(RecognitionEventArgs.Result))
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
		case ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech, Display, TEXT("%s: Task finished with result: %s"), *FString(__func__), *GetLastRecognizedString());
			RecognitionCompleted.Broadcast(GetLastRecognizedString());
			break;

		case ResultReason::RecognizingSpeech:
			RecognitionUpdated.Broadcast(GetLastRecognizedString());
			break;
	}
}