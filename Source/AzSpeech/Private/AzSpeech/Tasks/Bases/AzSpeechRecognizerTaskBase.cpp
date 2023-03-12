// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechRecognizerTaskBase.h"
#include "AzSpeech/Runnables/AzSpeechRecognitionRunnable.h"
#include "LogAzSpeech.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechRecognizerTaskBase)
#endif

const FString UAzSpeechRecognizerTaskBase::GetRecognizedString() const
{
	FScopeLock Lock(&Mutex);

	if (RecognizedText.empty())
	{
		return FString();
	}

	return UTF8_TO_TCHAR(RecognizedText.c_str());
}

void UAzSpeechRecognizerTaskBase::StartRecognitionWork(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig)
{
	RunnableTask = MakeShared<FAzSpeechRecognitionRunnable>(this, InAudioConfig);

	if (!RunnableTask)
	{
		SetReadyToDestroy();
		return;
	}
	
	RunnableTask->StartAzSpeechRunnableTask();
}

void UAzSpeechRecognizerTaskBase::BroadcastFinalResult()
{
	FScopeLock Lock(&Mutex);

	if (!UAzSpeechTaskStatus::IsTaskActive(this))
	{
		return;
	}

	Super::BroadcastFinalResult();

	RecognitionCompleted.Broadcast(GetRecognizedString());
}

void UAzSpeechRecognizerTaskBase::OnRecognitionUpdated(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognitionResult>& LastResult)
{
	check(IsInGameThread());

	if (!UAzSpeechTaskStatus::IsTaskStillValid(this) || !LastResult)
	{
		if (!UAzSpeechTaskStatus::IsTaskReadyToDestroy(this))
		{
			SetReadyToDestroy();
		}

		return;
	}
	
	FScopeLock Lock(&Mutex);

	RecognizedText = LastResult->Text;

	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current recognized text: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *GetRecognizedString());
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current duration: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__),LastResult->Duration());
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current offset: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), LastResult->Offset());
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current reason code: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), static_cast<int32>(LastResult->Reason));
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current result id: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), UTF8_TO_TCHAR(LastResult->ResultId.c_str()));

	RecognitionUpdated.Broadcast(GetRecognizedString());
}