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
		SetReadyToDestroy();
		return;
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&]
	{
		const TFuture<void> StopTaskWork = Async(EAsyncExecution::Thread, [=]() -> void
		{
			RecognitionCompleted.RemoveAll(this);
			RecognitionUpdated.RemoveAll(this);
			
			return RecognizerObject->StopContinuousRecognitionAsync().get();
		});

		StopTaskWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout()));

		if (IsValid(this))
		{
			AsyncTask(ENamedThreads::GameThread, [this] { SetReadyToDestroy(); });
		}
	});
}

void UAzSpeechRecognizerTaskBase::EnableContinuousRecognition()
{
	if (!RecognizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s - Trying to enable continuos recognition with invalid recognizer"), *FString(__func__));

		StopAzSpeechTask();
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s called"), *FString(__func__));
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
		UE_LOG(LogAzSpeech, Error, TEXT("%s - Trying to disable continuos recognition with invalid recognizer"), *FString(__func__));

		StopAzSpeechTask();
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s called"), *FString(__func__));
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

std::string UAzSpeechRecognizerTaskBase::StartContinuousRecognition()
{
	if (RecognitionUpdated.IsBound() || RecognitionCompleted.IsBound())
	{
		RecognizerObject->Recognizing.Connect([this](const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
		{
			OnContinuousRecognitionUpdated(RecognitionEventArgs);
		});
	}

	RecognizerObject->StartContinuousRecognitionAsync().get();
	
	return "CONTINUOUS_RECOGNITION";
}

void UAzSpeechRecognizerTaskBase::OnContinuousRecognitionUpdated(const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
{
	if (!CanBroadcast())
	{
		return;
	}

	if (const auto RecognitionResult = RecognitionEventArgs.Result;
		AzSpeech::Internal::ProcessRecognitionResult(RecognitionResult))
	{
		LastRecognizedString = RecognitionResult->Text;

		if (RecognitionResult->Reason == ResultReason::RecognizingSpeech)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - Continuous Recognition Updated: Recognized String: %s"), *GetLastRecognizedString());
			RecognitionUpdated.Broadcast(GetLastRecognizedString());		
		}
		else if (RecognitionResult->Reason == ResultReason::RecognizedSpeech)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - Continuous Recognition Completed: Recognized String: %s"), *GetLastRecognizedString());
			RecognitionCompleted.Broadcast(GetLastRecognizedString());
		}
	}
	else
	{
		AzSpeech::Internal::ProcessRecognitionResult(RecognitionEventArgs.Result);
		StopAzSpeechTask();
	}
}
