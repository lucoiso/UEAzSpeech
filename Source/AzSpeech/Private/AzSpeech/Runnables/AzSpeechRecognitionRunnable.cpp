// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/AzSpeechRecognitionRunnable.h"
#include "AzSpeech/Tasks/Bases/AzSpeechRecognizerTaskBase.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>
#include <Misc/ScopeTryLock.h>

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_phrase_list_grammar.h>
THIRD_PARTY_INCLUDES_END

FAzSpeechRecognitionRunnable::FAzSpeechRecognitionRunnable(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig) : Super(InOwningTask, InAudioConfig)
{
}

uint32 FAzSpeechRecognitionRunnable::Run()
{
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
	if (!UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
	{
		return 0u;
	}

	const std::future<void> Future = SpeechRecognizer->StartContinuousRecognitionAsync();

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Starting recognition"), *GetThreadName(), *FString(__func__));
	if (Future.wait_for(GetTaskTimeout()); Future.valid())
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Recognition started."), *GetThreadName(), *FString(__func__));
	}
	else
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Recognition failed to start."), *GetThreadName(), *FString(__func__));
		AsyncTask(ENamedThreads::GameThread,
			[RecognizerTask]
			{
				RecognizerTask->RecognitionFailed.Broadcast();
			}
		);

		return 0u;
	}

	AsyncTask(ENamedThreads::GameThread,
		[RecognizerTask]
		{
			RecognizerTask->RecognitionStarted.Broadcast();
		}
	);

	const float SleepTime = GetThreadUpdateInterval();
	while (!IsPendingStop())
	{
		FPlatformProcess::Sleep(SleepTime);
	}

	return 1u;
}

void FAzSpeechRecognitionRunnable::Exit()
{
	FScopeTryLock Lock(&Mutex);

	Super::Stop();

	if (Lock.IsLocked() && SpeechRecognizer)
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

const bool FAzSpeechRecognitionRunnable::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) const
{
	if (!Super::ApplySDKSettings(InConfig))
	{
		return false;
	}

	UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();
	if (!IsValid(RecognizerTask))
	{
		return false;
	}

	InConfig->SetProperty(Microsoft::CognitiveServices::Speech::PropertyId::Speech_SegmentationSilenceTimeoutMs, TCHAR_TO_UTF8(*FString::FromInt(RecognizerTask->GetRecognitionOptions().SegmentationSilenceTimeoutMs)));
	InConfig->SetProperty(Microsoft::CognitiveServices::Speech::PropertyId::SpeechServiceConnection_InitialSilenceTimeoutMs, TCHAR_TO_UTF8(*FString::FromInt(RecognizerTask->GetRecognitionOptions().InitialSilenceTimeoutMs)));

	InConfig->SetOutputFormat(GetOutputFormat());

	InsertProfanityFilterProperty(RecognizerTask->GetRecognitionOptions().ProfanityFilter, InConfig);

	if (RecognizerTask->GetRecognitionOptions().bUseLanguageIdentification)
	{
		InsertLanguageIdentificationProperty(RecognizerTask->GetRecognitionOptions().LanguageIdentificationMode, InConfig);
		return true;
	}

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Using language: %s"), *GetThreadName(), *FString(__func__), *RecognizerTask->GetRecognitionOptions().Locale.ToString());

	const std::string UsedLang = TCHAR_TO_UTF8(*RecognizerTask->GetRecognitionOptions().Locale.ToString());
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
	if (!UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
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

	if (RecognizerTask->GetRecognitionOptions().bUseLanguageIdentification)
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
	UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();
	if (!IsSpeechRecognizerValid() || !UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
	{
		return false;
	}

	SpeechRecognizer->Recognizing.Connect(
		[this, RecognizerTask](const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
		{
			if (!UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
			{
				StopAzSpeechRunnableTask();
			}
			else
			{
				AsyncTask(ENamedThreads::GameThread,
					[RecognizerTask, Result = RecognitionEventArgs.Result]
					{
						RecognizerTask->OnRecognitionUpdated(Result);
					}
				);
			}
		}
	);

	SpeechRecognizer->Recognized.Connect(
		[this, RecognizerTask](const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs)
		{
			if (!UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
			{
				StopAzSpeechRunnableTask();
				return;
			}

			const bool bValidResult = ProcessRecognitionResult(RecognitionEventArgs.Result);
			if (!bValidResult)
			{
				AsyncTask(ENamedThreads::GameThread, 
					[RecognizerTask] 
					{
						RecognizerTask->RecognitionFailed.Broadcast(); 
					}
				);
			}
			else
			{
				AsyncTask(ENamedThreads::GameThread,
					[RecognizerTask, Result = RecognitionEventArgs.Result]
					{
						RecognizerTask->OnRecognitionUpdated(Result);
						RecognizerTask->BroadcastFinalResult();
					}
				);
			}

			StopAzSpeechRunnableTask();
		}
	);
	
	return true;
}

bool FAzSpeechRecognitionRunnable::InsertPhraseList() const
{
	UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();
	if (!IsSpeechRecognizerValid() || !UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
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

	switch (LastResult->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizingSpeech:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Task running. Reason: RecognizingSpeech"), *GetThreadName(), *FString(__func__));
			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech:
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Task completed. Reason: RecognizedSpeech"), *GetThreadName(), *FString(__func__));
			break;

		case Microsoft::CognitiveServices::Speech::ResultReason::NoMatch:
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Task failed. Reason: NoMatch"), *GetThreadName(), *FString(__func__));
			bOutput = false;
			break;

		default:
			break;
	}

	if (LastResult->Reason == Microsoft::CognitiveServices::Speech::ResultReason::Canceled)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Task failed. Reason: Canceled"), *GetThreadName(), *FString(__func__));

		bOutput = false;
		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::CancellationDetails::FromResult(LastResult);

		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Cancellation Reason: %s"), *GetThreadName(), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));
		if (CancellationDetails->Reason == Microsoft::CognitiveServices::Speech::CancellationReason::Error)
		{
			ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
		}
	}

	return bOutput;
}

const std::vector<std::string> FAzSpeechRecognitionRunnable::GetCandidateLanguages() const
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Getting candidate languages"), *GetThreadName(), *FString(__func__));

	std::vector<std::string> Output;

	UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();
	if (!UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
	{
		return Output;
	}

	const uint8 MaxAllowedCandidateLanguages = RecognizerTask->GetRecognitionOptions().GetMaxAllowedCandidateLanguages();
	for (const FName& Iterator : RecognizerTask->GetRecognitionOptions().CandidateLanguages)
	{
		if (AzSpeech::Internal::HasEmptyParam(Iterator))
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Found empty candidate language in settings"), *FString(__func__));
			continue;
		}

		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Using language %s as candidate"), *GetThreadName(), *FString(__func__), *Iterator.ToString());

		Output.push_back(TCHAR_TO_UTF8(*Iterator.ToString()));
		if (Output.size() > MaxAllowedCandidateLanguages)
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: You can only include up to 4 languages for at-start LID and up to 10 languages for continuous LID."), *GetThreadName(), *FString(__func__));
			Output.resize(MaxAllowedCandidateLanguages);
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

const Microsoft::CognitiveServices::Speech::OutputFormat FAzSpeechRecognitionRunnable::GetOutputFormat() const
{	
	if (UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask(); UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
	{
		switch (RecognizerTask->GetRecognitionOptions().SpeechRecognitionOutputFormat)
		{
			case EAzSpeechRecognitionOutputFormat::Simple:
				return Microsoft::CognitiveServices::Speech::OutputFormat::Simple;

			case EAzSpeechRecognitionOutputFormat::Detailed:
				return Microsoft::CognitiveServices::Speech::OutputFormat::Detailed;

			default:
				break;
		}
	}

	return Microsoft::CognitiveServices::Speech::OutputFormat::Detailed;
}
