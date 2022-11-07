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
	
	if (RecognitionCompleted.IsBound())
	{
		RecognitionCompleted.RemoveAll(this);
	}

	if (RecognitionUpdated.IsBound())
	{
		RecognitionUpdated.RemoveAll(this);
	}

	if (RecognizerObject->Recognizing.IsConnected())
	{
		RecognizerObject->Recognizing.DisconnectAll();
	}
	
	if (bContinuousRecognition)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, FuncName = __func__]
		{
			if (RecognizerObject)
			{
				UE_LOG(LogAzSpeech, Display, TEXT("%s - Trying to stop current recognizer task..."), *FString(FuncName));
				
				switch (RecognizerObject->StopContinuousRecognitionAsync().wait_for(std::chrono::duration<float>(AzSpeech::Internal::GetTimeout())))
				{
					case std::future_status::ready :
						UE_LOG(LogAzSpeech, Display, TEXT("%s - Stop finished with status: Ready"), *FString(FuncName));
						break;

					case std::future_status::deferred:
						UE_LOG(LogAzSpeech, Display, TEXT("%s - Stop finished with status: Deferred"), *FString(FuncName));
						break;

					case std::future_status::timeout:
						UE_LOG(LogAzSpeech, Display, TEXT("%s - Stop finished with status: Time Out"), *FString(FuncName));
						break;

					default: break;
				}

				RecognizerObject.reset();
			}
		});
	}
}

void UAzSpeechRecognizerTaskBase::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
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
	if (const auto RecognitionResult = RecognitionEventArgs.Result;
		AzSpeech::Internal::ProcessRecognitionResult(RecognitionResult))
	{
		LastRecognizedString = RecognitionResult->Text;

		if (RecognitionResult->Reason == ResultReason::RecognizingSpeech)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("%s: Updated String: %s"), *FString(__func__), *GetLastRecognizedString());
			
			if (CanBroadcast())
			{
				AsyncTask(ENamedThreads::GameThread, [=]() { RecognitionUpdated.Broadcast(GetLastRecognizedString()); });
			}
		}
		else if (RecognitionResult->Reason == ResultReason::RecognizedSpeech)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("%s: Final String: %s"), *FString(__func__), *GetLastRecognizedString());

			if (CanBroadcast())
			{
				AsyncTask(ENamedThreads::GameThread, [=]() { RecognitionCompleted.Broadcast(GetLastRecognizedString()); });
			}
		}
	}
	else
	{
		AzSpeech::Internal::ProcessRecognitionResult(RecognitionEventArgs.Result);
		StopAzSpeechTask();
	}
}
