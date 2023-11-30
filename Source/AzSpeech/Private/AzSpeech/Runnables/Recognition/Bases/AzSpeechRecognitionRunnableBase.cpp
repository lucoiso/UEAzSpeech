// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/Recognition/Bases/AzSpeechRecognitionRunnableBase.h"
#include "AzSpeech/Tasks/Recognition/Bases/AzSpeechRecognizerTaskBase.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>
#include <Misc/ScopeTryLock.h>

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_phrase_list_grammar.h>
THIRD_PARTY_INCLUDES_END

namespace MicrosoftSpeech = Microsoft::CognitiveServices::Speech;

FAzSpeechRecognitionRunnableBase::FAzSpeechRecognitionRunnableBase(UAzSpeechTaskBase* const InOwningTask, const std::shared_ptr<MicrosoftSpeech::Audio::AudioConfig>& InAudioConfig)
    : FAzSpeechRunnableBase(InOwningTask, InAudioConfig)
{
}

uint32 FAzSpeechRecognitionRunnableBase::Run()
{
    return FAzSpeechRunnableBase::Run();
}

void FAzSpeechRecognitionRunnableBase::Exit()
{
    FScopeTryLock Lock(&Mutex);

    FAzSpeechRunnableBase::Exit();

    if (Lock.IsLocked() && SpeechRecognizer)
    {
        SpeechRecognizer->Recognized.DisconnectAll();
        SpeechRecognizer->Recognizing.DisconnectAll();
        SpeechRecognizer->SessionStarted.DisconnectAll();
    }

    SpeechRecognizer = nullptr;
}

const bool FAzSpeechRecognitionRunnableBase::IsSpeechRecognizerValid() const
{
    if (!SpeechRecognizer)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Invalid recognizer"), *GetThreadName(), *FString(__func__));
    }

    return SpeechRecognizer != nullptr;
}

UAzSpeechRecognizerTaskBase* FAzSpeechRecognitionRunnableBase::GetOwningRecognizerTask() const
{
    if (!GetOwningTask())
    {
        return nullptr;
    }

    return Cast<UAzSpeechRecognizerTaskBase>(GetOwningTask());
}

const std::vector<std::string> FAzSpeechRecognitionRunnableBase::GetCandidateLanguages() const
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
            UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: You can only include up to 4 languages for at-start LID and up to 10 languages for continuous LID."), *GetThreadName(), *FString(__func__));
            Output.resize(MaxAllowedCandidateLanguages);
            break;
        }
    }

    return Output;
}

const TArray<FString> FAzSpeechRecognitionRunnableBase::GetPhraseListFromGroup(const FName& InGroup) const
{
    if (const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get())
    {
        return AzSpeech::Internal::GetDataFromMapGroup<TArray<FString>, FAzSpeechPhraseListMap>(InGroup, Settings->PhraseListMap);
    }

    return TArray<FString>();
}

const MicrosoftSpeech::OutputFormat FAzSpeechRecognitionRunnableBase::GetOutputFormat() const
{
    if (UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask(); UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
    {
        switch (RecognizerTask->GetRecognitionOptions().SpeechRecognitionOutputFormat)
        {
        case EAzSpeechRecognitionOutputFormat::Simple:
            return MicrosoftSpeech::OutputFormat::Simple;

        case EAzSpeechRecognitionOutputFormat::Detailed:
            return MicrosoftSpeech::OutputFormat::Detailed;

        default:
            break;
        }
    }

    return MicrosoftSpeech::OutputFormat::Detailed;
}

const bool FAzSpeechRecognitionRunnableBase::ApplySDKSettings(const std::shared_ptr<MicrosoftSpeech::SpeechConfig>& InConfig) const
{
    if (!FAzSpeechRunnableBase::ApplySDKSettings(InConfig))
    {
        return false;
    }

    UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();
    if (!IsValid(RecognizerTask))
    {
        return false;
    }

    InConfig->SetProperty(MicrosoftSpeech::PropertyId::Speech_SegmentationSilenceTimeoutMs, TCHAR_TO_UTF8(*FString::FromInt(RecognizerTask->GetRecognitionOptions().SegmentationSilenceTimeoutMs)));
    InConfig->SetProperty(MicrosoftSpeech::PropertyId::SpeechServiceConnection_InitialSilenceTimeoutMs, TCHAR_TO_UTF8(*FString::FromInt(RecognizerTask->GetRecognitionOptions().InitialSilenceTimeoutMs)));

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

bool FAzSpeechRecognitionRunnableBase::InitializeAzureObject()
{
    if (!FAzSpeechRunnableBase::InitializeAzureObject())
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

    const auto TaskAudioConfig = GetAudioConfig();
    if (!TaskAudioConfig)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Invalid audio config"), *GetThreadName(), *FString(__func__));
        return false;
    }

    if (RecognizerTask->GetRecognitionOptions().bUseLanguageIdentification)
    {
        const std::vector<std::string> Candidates = GetCandidateLanguages();

        if (Candidates.empty())
        {
            UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Task failed. Result: Invalid candidate languages"), *GetThreadName(), *FString(__func__));
            return false;
        }

        SpeechRecognizer = MicrosoftSpeech::SpeechRecognizer::FromConfig(SpeechConfig, MicrosoftSpeech::AutoDetectSourceLanguageConfig::FromLanguages(Candidates), TaskAudioConfig);
    }
    else
    {
        SpeechRecognizer = MicrosoftSpeech::SpeechRecognizer::FromConfig(SpeechConfig, TaskAudioConfig);
    }

    return InsertPhraseList() && ConnectRecognitionStartedSignals() && ConnectRecognitionUpdatedSignals();
}

bool FAzSpeechRecognitionRunnableBase::ConnectRecognitionStartedSignals()
{
    UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();
    if (!IsSpeechRecognizerValid() || !UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
    {
        return false;
    }

    SpeechRecognizer->SessionStarted.Connect(
        [this, RecognizerTask]([[maybe_unused]] const MicrosoftSpeech::SessionEventArgs& SessionEventArgs)
        {
            if (!UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
            {
                StopAzSpeechRunnableTask();
            }
            else
            {
                AsyncTask(ENamedThreads::GameThread,
                    [RecognizerTask]
                    {
                        RecognizerTask->RecognitionStarted.Broadcast();
                    }
                );
            }
        }
    );

    return true;
}

bool FAzSpeechRecognitionRunnableBase::ConnectRecognitionUpdatedSignals()
{
    UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();
    if (!IsSpeechRecognizerValid() || !UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
    {
        return false;
    }

    SpeechRecognizer->Recognizing.Connect(
        [this, RecognizerTask](const MicrosoftSpeech::SpeechRecognitionEventArgs& RecognitionEventArgs)
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
        [this, RecognizerTask](const MicrosoftSpeech::SpeechRecognitionEventArgs& RecognitionEventArgs)
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

bool FAzSpeechRecognitionRunnableBase::InsertPhraseList() const
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

    const auto PhraseListGrammar = MicrosoftSpeech::PhraseListGrammar::FromRecognizer(SpeechRecognizer);
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

bool FAzSpeechRecognitionRunnableBase::ProcessRecognitionResult(const std::shared_ptr<MicrosoftSpeech::SpeechRecognitionResult>& LastResult)
{
    bool bOutput = true;

    switch (LastResult->Reason)
    {
    case MicrosoftSpeech::ResultReason::RecognizingSpeech:
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Task running. Reason: RecognizingSpeech"), *GetThreadName(), *FString(__func__));
        break;

    case MicrosoftSpeech::ResultReason::RecognizedSpeech:
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Task completed. Reason: RecognizedSpeech"), *GetThreadName(), *FString(__func__));
        break;

    case MicrosoftSpeech::ResultReason::NoMatch:
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Task failed. Reason: NoMatch"), *GetThreadName(), *FString(__func__));
        bOutput = false;
        break;

    default:
        break;
    }

    if (LastResult->Reason == MicrosoftSpeech::ResultReason::Canceled)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Task failed. Reason: Canceled"), *GetThreadName(), *FString(__func__));

        bOutput = false;
        const auto CancellationDetails = MicrosoftSpeech::CancellationDetails::FromResult(LastResult);

        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Cancellation Reason: %s"), *GetThreadName(), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));
        if (CancellationDetails->Reason == MicrosoftSpeech::CancellationReason::Error)
        {
            ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
        }
    }

    return bOutput;
}