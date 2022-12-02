// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/AzSpeechRecognitionRunnable.h"
#include "AzSpeech/Tasks/Bases/AzSpeechRecognizerTaskBase.h"
#include <Async/Async.h>

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_phrase_list_grammar.h>
THIRD_PARTY_INCLUDES_END

FAzSpeechRecognitionRunnable::FAzSpeechRecognitionRunnable(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig) : Super(InOwningTask, InAudioConfig)
{
}

const std::shared_ptr<Microsoft::CognitiveServices::Speech::Recognizer> FAzSpeechRecognitionRunnable::GetRecognizer() const
{
	return SpeechRecognizer->shared_from_this();
}

uint32 FAzSpeechRecognitionRunnable::Run()
{
#if !UE_BUILD_SHIPPING
	const int64 StartTime = FAzSpeechRunnableBase::GetTimeInMilliseconds();
#endif

	if (Super::Run() == 0u)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Run returned 0"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return 0u;
	}
	
	if (!SpeechRecognizer)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid recognizer"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return 0u;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = Cast<UAzSpeechRecognizerTaskBase>(OwningTask);

	if (!UAzSpeechTaskBase::IsTaskStillValid(RecognizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return 0u;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Starting recognition"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
	SpeechRecognizer->StartContinuousRecognitionAsync().wait_for(GetTaskTimeout());
	
	if (RecognizerTask->RecognitionStarted.IsBound())
	{
		AsyncTask(ENamedThreads::GameThread, [RecognizerTask] 
		{ 
			RecognizerTask->RecognitionStarted.Broadcast();
			RecognizerTask->RecognitionStarted.Clear();
		});
	}

#if !UE_BUILD_SHIPPING
	const int64 ActivationDelay = FAzSpeechRunnableBase::GetTimeInMilliseconds() - StartTime;
#endif

	const float SleepTime = AzSpeech::Internal::GetThreadUpdateInterval();
	
	while (!IsPendingStop())
	{
#if !UE_BUILD_SHIPPING
		FAzSpeechRunnableBase::PrintDebugInformation(RecognizerTask, StartTime, ActivationDelay, SleepTime);
#endif
		FPlatformProcess::Sleep(SleepTime);
	}
	
	return 1u;
}

void FAzSpeechRecognitionRunnable::Exit()
{
	Super::Exit();

	if (SpeechRecognizer)
	{
		SpeechRecognizer->StopContinuousRecognitionAsync().wait_for(GetTaskTimeout());
	}

	SpeechRecognizer = nullptr;
}

void FAzSpeechRecognitionRunnable::ClearSignals()
{
	Super::ClearSignals();

	if (!SpeechRecognizer)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid recognizer"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return;
	}
	
	SignalDisconnecter_T(SpeechRecognizer->Recognizing);
	SignalDisconnecter_T(SpeechRecognizer->Recognized);
}

void FAzSpeechRecognitionRunnable::RemoveBindings()
{
	Super::RemoveBindings();
	
	UAzSpeechRecognizerTaskBase* const RecognizerTask = Cast<UAzSpeechRecognizerTaskBase>(OwningTask);
		
	if (!IsValid(RecognizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return;
	}

	DelegateDisconnecter_T(RecognizerTask->RecognitionStarted);
	DelegateDisconnecter_T(RecognizerTask->RecognitionUpdated);
}

bool FAzSpeechRecognitionRunnable::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) const
{
	if (!Super::ApplySDKSettings(InConfig))
	{
		return false;
	}
	
	if (OwningTask->IsUsingAutoLanguage())
	{
		return true;
	}
	
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Using language: %s"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), *OwningTask->GetLanguageID());

	const std::string UsedLang = TCHAR_TO_UTF8(*OwningTask->GetLanguageID());
	InConfig->SetSpeechRecognitionLanguage(UsedLang);

	return !AzSpeech::Internal::HasEmptyParam(UsedLang);
}

bool FAzSpeechRecognitionRunnable::InitializeAzureObject()
{
	if (!Super::InitializeAzureObject())
	{
		return false;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = Cast<UAzSpeechRecognizerTaskBase>(OwningTask);

	if (!IsValid(RecognizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Creating recognizer object"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

	const auto SpeechConfig = CreateSpeechConfig();

	if (!SpeechConfig)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid speech config"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	ApplySDKSettings(SpeechConfig);

	if (RecognizerTask->IsUsingAutoLanguage())
	{
		const std::vector<std::string> Candidates = AzSpeech::Internal::GetCandidateLanguages(*RecognizerTask->GetTaskName(), RecognizerTask->GetUniqueID());
		if (Candidates.empty())
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Task failed. Result: Invalid candidate languages"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
			
			return false;
		}

		SpeechRecognizer = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(SpeechConfig, Microsoft::CognitiveServices::Speech::AutoDetectSourceLanguageConfig::FromLanguages(Candidates), AudioConfig);
	}
	else
	{
		SpeechRecognizer = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(SpeechConfig, AudioConfig);
	}

	return InsertPhraseList() && ConnectRecognitionSignals();
}

bool FAzSpeechRecognitionRunnable::ConnectRecognitionSignals()
{
	if (!SpeechRecognizer)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid recognizer"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = Cast<UAzSpeechRecognizerTaskBase>(OwningTask);
	if (!IsValid(RecognizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));	
		return false;
	}
	
	const auto RecognitionUpdate_Lambda = [this, RecognizerTask](const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
	{
		if (!IsValid(RecognizerTask) || !ProcessRecognitionResult(RecognitionEventArgs.Result))
		{
			StopAzSpeechRunnableTask();
			return;
		}
		
		AsyncTask(ENamedThreads::GameThread, [RecognizerTask, TaskResult = RecognitionEventArgs.Result] { RecognizerTask->OnRecognitionUpdated(TaskResult); });
	};

	SpeechRecognizer->Recognized.Connect(RecognitionUpdate_Lambda);
	SpeechRecognizer->Recognizing.Connect(RecognitionUpdate_Lambda);
	
	return true;
}

bool FAzSpeechRecognitionRunnable::InsertPhraseList()
{
	if (!SpeechRecognizer)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid recognizer"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = Cast<UAzSpeechRecognizerTaskBase>(OwningTask);
	if (!IsValid(RecognizerTask))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid owning task"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	if (AzSpeech::Internal::HasEmptyParam(RecognizerTask->PhraseListGroup))
	{
		return true;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Inserting Phrase List Data in Recognition Object"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

	const auto PhraseListGrammar = Microsoft::CognitiveServices::Speech::PhraseListGrammar::FromRecognizer(SpeechRecognizer);
	if (!PhraseListGrammar)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid phrase list grammar"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
		return false;
	}

	for (const FString& PhraseListData : AzSpeech::Internal::GetPhraseListFromGroup(RecognizerTask->PhraseListGroup))
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Inserting Phrase List Data %s to Phrase List Grammar"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), *PhraseListData);

		PhraseListGrammar->AddPhrase(TCHAR_TO_UTF8(*PhraseListData));
	}

	return true;
}


bool FAzSpeechRecognitionRunnable::ProcessRecognitionResult(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognitionResult>& LastResult)
{
	bool bOutput = true;
	bool bFinishTask = false;

	switch (LastResult->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizingSpeech:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Task running. Reason: RecognizingSpeech"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
			bOutput = true;
			bFinishTask = false;
			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Task completed. Reason: RecognizedSpeech"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
			bOutput = true;
			bFinishTask = true;
			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::NoMatch:
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Task failed. Reason: NoMatch"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));
			bOutput = false;
			bFinishTask = true;
			break;

		default:
			break;
	}

	if (LastResult->Reason == Microsoft::CognitiveServices::Speech::ResultReason::Canceled)
	{
		bOutput = false;
		bFinishTask = true;

		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Task failed. Reason: Canceled"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__));

		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::CancellationDetails::FromResult(LastResult);

		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Cancellation Reason: %s"), *OwningTask->GetTaskName(), OwningTask->GetUniqueID(), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

		if (CancellationDetails->Reason == Microsoft::CognitiveServices::Speech::CancellationReason::Error)
		{
			ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
		}
	}

	if (bFinishTask)
	{
		StopAzSpeechRunnableTask();
	}

	return bOutput;
}