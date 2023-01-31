// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/AzSpeechRecognitionRunnable.h"
#include "AzSpeech/Tasks/Bases/AzSpeechRecognizerTaskBase.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_phrase_list_grammar.h>
THIRD_PARTY_INCLUDES_END

FAzSpeechRecognitionRunnable::FAzSpeechRecognitionRunnable(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig) : Super(InOwningTask, InAudioConfig)
{
}

uint32 FAzSpeechRecognitionRunnable::Run()
{
#if !UE_BUILD_SHIPPING
	const int64 StartTime = GetTimeInMilliseconds();
#endif

	if (Super::Run() == 0u)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Run returned 0"), *GetThreadName(), *FString(__func__));
		return 0u;
	}
	
	if (!IsSpeechRecognizerValid())
	{
		return 0u;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();

	if (!UAzSpeechTaskBase::IsTaskStillValid(RecognizerTask))
	{
		return 0u;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Starting recognition"), *GetThreadName(), *FString(__func__));
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
	const int64 ActivationDelay = GetTimeInMilliseconds() - StartTime;
#endif

	const float SleepTime = GetThreadUpdateInterval();
	
	while (!IsPendingStop())
	{
#if !UE_BUILD_SHIPPING
		PrintDebugInformation(StartTime, ActivationDelay, SleepTime);
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

const bool FAzSpeechRecognitionRunnable::IsSpeechRecognizerValid() const
{
	if (!SpeechRecognizer)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Invalid recognizer"), *GetThreadName(), *FString(__func__));
	}

	return SpeechRecognizer != nullptr;
}

UAzSpeechRecognizerTaskBase* FAzSpeechRecognitionRunnable::GetOwningRecognizerTask() const
{
	if (!GetOwningTask())
	{
		return nullptr;
	}

	return Cast<UAzSpeechRecognizerTaskBase>(GetOwningTask());
}

void FAzSpeechRecognitionRunnable::ClearSignals()
{
	Super::ClearSignals();

	if (!IsSpeechRecognizerValid())
	{
		return;
	}
	
	SignalDisconnecter_T(SpeechRecognizer->Recognizing);
	SignalDisconnecter_T(SpeechRecognizer->Recognized);
}

void FAzSpeechRecognitionRunnable::RemoveBindings()
{
	Super::RemoveBindings();
	
	UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();

	if (!UAzSpeechTaskBase::IsTaskStillValid(RecognizerTask))
	{
		return;
	}

	DelegateDisconnecter_T(RecognizerTask->RecognitionStarted);
	DelegateDisconnecter_T(RecognizerTask->RecognitionUpdated);
}

const bool FAzSpeechRecognitionRunnable::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) const
{
	if (!Super::ApplySDKSettings(InConfig))
	{
		return false;
	}
	
	if (GetOwningTask()->IsUsingAutoLanguage())
	{
		return true;
	}
	
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Using language: %s"), *GetThreadName(), *FString(__func__), *GetOwningTask()->GetLanguageID());

	const std::string UsedLang = TCHAR_TO_UTF8(*GetOwningTask()->GetLanguageID());
	InConfig->SetSpeechRecognitionLanguage(UsedLang);

	return !AzSpeech::Internal::HasEmptyParam(UsedLang);
}

bool FAzSpeechRecognitionRunnable::InitializeAzureObject()
{
	if (!Super::InitializeAzureObject())
	{
		return false;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();

	if (!UAzSpeechTaskBase::IsTaskStillValid(RecognizerTask))
	{
		return false;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Creating recognizer object"), *GetThreadName(), *FString(__func__));

	const auto SpeechConfig = CreateSpeechConfig();

	if (!SpeechConfig)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Invalid speech config"), *GetThreadName(), *FString(__func__));
		return false;
	}

	ApplySDKSettings(SpeechConfig);

	if (RecognizerTask->IsUsingAutoLanguage())
	{
		const std::vector<std::string> Candidates = GetCandidateLanguages();
		if (Candidates.empty())
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Task failed. Result: Invalid candidate languages"), *GetThreadName(), *FString(__func__));
			
			return false;
		}

		SpeechRecognizer = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(SpeechConfig, Microsoft::CognitiveServices::Speech::AutoDetectSourceLanguageConfig::FromLanguages(Candidates), GetAudioConfig());
	}
	else
	{
		SpeechRecognizer = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(SpeechConfig, GetAudioConfig());
	}

	return InsertPhraseList() && ConnectRecognitionSignals();
}

bool FAzSpeechRecognitionRunnable::ConnectRecognitionSignals()
{
	if (!IsSpeechRecognizerValid())
	{
		return false;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();

	if (!UAzSpeechTaskBase::IsTaskStillValid(RecognizerTask))
	{
		return false;
	}
	
	const auto RecognitionUpdate_Lambda = [this, RecognizerTask](const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
	{
		if (!UAzSpeechTaskBase::IsTaskStillValid(RecognizerTask) || !ProcessRecognitionResult(RecognitionEventArgs.Result))
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
	if (!IsSpeechRecognizerValid())
	{
		return false;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();

	if (!UAzSpeechTaskBase::IsTaskStillValid(RecognizerTask))
	{
		return false;
	}

	if (AzSpeech::Internal::HasEmptyParam(RecognizerTask->PhraseListGroup))
	{
		return true;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Inserting Phrase List Data in Recognition Object"), *GetThreadName(), *FString(__func__));

	const auto PhraseListGrammar = Microsoft::CognitiveServices::Speech::PhraseListGrammar::FromRecognizer(SpeechRecognizer);
	if (!PhraseListGrammar)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Invalid phrase list grammar"), *GetThreadName(), *FString(__func__));
		return false;
	}

	for (const FString& PhraseListData : GetPhraseListFromGroup(RecognizerTask->PhraseListGroup))
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Inserting Phrase List Data %s to Phrase List Grammar"), *GetThreadName(), *FString(__func__), *PhraseListData);

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
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Task running. Reason: RecognizingSpeech"), *GetThreadName(), *FString(__func__));
			bOutput = true;
			bFinishTask = false;
			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Task completed. Reason: RecognizedSpeech"), *GetThreadName(), *FString(__func__));
			bOutput = true;
			bFinishTask = true;
			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::NoMatch:
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Task failed. Reason: NoMatch"), *GetThreadName(), *FString(__func__));
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

		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Task failed. Reason: Canceled"), *GetThreadName(), *FString(__func__));

		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::CancellationDetails::FromResult(LastResult);

		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Cancellation Reason: %s"), *GetThreadName(), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

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

const std::vector<std::string> FAzSpeechRecognitionRunnable::GetCandidateLanguages() const
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Getting candidate languages"), *GetThreadName(), *FString(__func__));

	std::vector<std::string> Output;

	const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get();
	for (const FString& Iterator : Settings->AutoCandidateLanguages)
	{
		if (AzSpeech::Internal::HasEmptyParam(Iterator))
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Found empty candidate language in settings"), *FString(__func__));
			continue;
		}

		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Using language %s as candidate"), *GetThreadName(), *FString(__func__), *Iterator);

		Output.push_back(TCHAR_TO_UTF8(*Iterator));

		if (Output.size() >= UAzSpeechSettings::MaxCandidateLanguages)
		{
			Output.resize(UAzSpeechSettings::MaxCandidateLanguages);
			break;
		}
	}

	return Output;
}

const TArray<FString> FAzSpeechRecognitionRunnable::GetPhraseListFromGroup(const FName& InGroup) const
{
	if (const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get())
	{
		return AzSpeech::Internal::GetDataFromMapGroup<TArray<FString>, FAzSpeechPhraseListMap>(InGroup, Settings->PhraseListMap);
	}

	return TArray<FString>();
}
