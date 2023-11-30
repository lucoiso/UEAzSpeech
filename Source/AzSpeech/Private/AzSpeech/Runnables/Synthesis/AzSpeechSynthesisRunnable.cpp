// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/Synthesis/AzSpeechSynthesisRunnable.h"
#include "AzSpeech/Tasks/Synthesis/Bases/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>
#include <Misc/ScopeTryLock.h>

namespace MicrosoftSpeech = Microsoft::CognitiveServices::Speech;

FAzSpeechSynthesisRunnable::FAzSpeechSynthesisRunnable(UAzSpeechTaskBase* const InOwningTask, const std::shared_ptr<MicrosoftSpeech::Audio::AudioConfig>& InAudioConfig)
    : FAzSpeechRunnableBase(InOwningTask, InAudioConfig)
{
}

uint32 FAzSpeechSynthesisRunnable::Run()
{
    if (FAzSpeechRunnableBase::Run() == 0u)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Run returned 0"), *GetThreadName(), *FString(__func__));
        return 0u;
    }

    if (!IsSpeechSynthesizerValid())
    {
        return 0u;
    }

    UAzSpeechSynthesizerTaskBase* const SynthesizerTask = GetOwningSynthesizerTask();
    if (!UAzSpeechTaskStatus::IsTaskStillValid(SynthesizerTask))
    {
        return 0u;
    }

    UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Thread: %s; Function: %s; Message: Using text: %s"), *GetThreadName(), *FString(__func__), *SynthesizerTask->GetSynthesisText());

    const std::string SynthesisStr = TCHAR_TO_UTF8(*SynthesizerTask->GetSynthesisText());
    std::future<std::shared_ptr<MicrosoftSpeech::SpeechSynthesisResult>> Future;
    if (SynthesizerTask->IsSSMLBased())
    {
        Future = SpeechSynthesizer->StartSpeakingSsmlAsync(SynthesisStr);
    }
    else
    {
        Future = SpeechSynthesizer->StartSpeakingTextAsync(SynthesisStr);
    }

    UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Starting synthesis."), *GetThreadName(), *FString(__func__));
    if (Future.wait_for(GetTaskTimeout()); Future.valid())
    {
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Synthesis started."), *GetThreadName(), *FString(__func__));
    }
    else
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Synthesis failed to start."), *GetThreadName(), *FString(__func__));
        AsyncTask(ENamedThreads::GameThread,
            [SynthesizerTask]
            {
                SynthesizerTask->SynthesisFailed.Broadcast();
            }
        );

        return 0u;
    }

    const float SleepTime = GetThreadUpdateInterval();
    while (!IsPendingStop())
    {
        FPlatformProcess::Sleep(SleepTime);
    }

    return 1u;
}

void FAzSpeechSynthesisRunnable::Exit()
{
    FScopeTryLock Lock(&Mutex);

    FAzSpeechRunnableBase::Exit();

    if (Lock.IsLocked() && SpeechSynthesizer)
    {
        SpeechSynthesizer->VisemeReceived.DisconnectAll();
        SpeechSynthesizer->SynthesisCanceled.DisconnectAll();
        SpeechSynthesizer->SynthesisCompleted.DisconnectAll();
        SpeechSynthesizer->SynthesisStarted.DisconnectAll();
        SpeechSynthesizer->Synthesizing.DisconnectAll();
    }

    SpeechSynthesizer = nullptr;
}

const bool FAzSpeechSynthesisRunnable::IsSpeechSynthesizerValid() const
{
    if (!SpeechSynthesizer)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Invalid synthesizer"), *GetThreadName(), *FString(__func__));
    }

    return SpeechSynthesizer != nullptr;
}

UAzSpeechSynthesizerTaskBase* FAzSpeechSynthesisRunnable::GetOwningSynthesizerTask() const
{
    if (!GetOwningTask())
    {
        return nullptr;
    }

    return Cast<UAzSpeechSynthesizerTaskBase>(GetOwningTask());
}

const bool FAzSpeechSynthesisRunnable::ApplySDKSettings(const std::shared_ptr<MicrosoftSpeech::SpeechConfig>& InConfig) const
{
    if (!FAzSpeechRunnableBase::ApplySDKSettings(InConfig))
    {
        return false;
    }

    UAzSpeechSynthesizerTaskBase* const SynthesizerTask = GetOwningSynthesizerTask();
    if (!UAzSpeechTaskStatus::IsTaskStillValid(SynthesizerTask))
    {
        return false;
    }

    InConfig->SetProperty("SpeechSynthesis_KeepConnectionAfterStopping", "false");
    InConfig->SetSpeechSynthesisOutputFormat(GetOutputFormat());

    InsertProfanityFilterProperty(SynthesizerTask->GetSynthesisOptions().ProfanityFilter, InConfig);

    if (SynthesizerTask->IsSSMLBased())
    {
        return true;
    }

    if (SynthesizerTask->GetSynthesisOptions().bUseLanguageIdentification)
    {
        InsertLanguageIdentificationProperty(SynthesizerTask->GetSynthesisOptions().LanguageIdentificationMode, InConfig);
        return true;
    }

    const std::string UsedLang = TCHAR_TO_UTF8(*SynthesizerTask->GetSynthesisOptions().Locale.ToString());
    const std::string UsedVoice = TCHAR_TO_UTF8(*SynthesizerTask->GetSynthesisOptions().Voice.ToString());

    UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Using language: %s"), *GetThreadName(), *FString(__func__), *SynthesizerTask->GetSynthesisOptions().Locale.ToString());
    InConfig->SetSpeechSynthesisLanguage(UsedLang);

    UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Using voice: %s"), *GetThreadName(), *FString(__func__), *SynthesizerTask->GetSynthesisOptions().Voice.ToString());
    InConfig->SetSpeechSynthesisVoiceName(UsedVoice);

    return true;
}

bool FAzSpeechSynthesisRunnable::InitializeAzureObject()
{
    if (!FAzSpeechRunnableBase::InitializeAzureObject())
    {
        return false;
    }

    UAzSpeechSynthesizerTaskBase* const SynthesizerTask = GetOwningSynthesizerTask();
    if (!UAzSpeechTaskStatus::IsTaskStillValid(SynthesizerTask))
    {
        return false;
    }

    UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Creating synthesizer object"), *GetThreadName(), *FString(__func__));

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

    if (!SynthesizerTask->IsSSMLBased() && SynthesizerTask->GetSynthesisOptions().bUseLanguageIdentification)
    {
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Initializing auto language detection"), *GetThreadName(), *FString(__func__));
        SpeechSynthesizer = MicrosoftSpeech::SpeechSynthesizer::FromConfig(SpeechConfig, MicrosoftSpeech::AutoDetectSourceLanguageConfig::FromOpenRange(), TaskAudioConfig);
    }
    else
    {
        SpeechSynthesizer = MicrosoftSpeech::SpeechSynthesizer::FromConfig(SpeechConfig, TaskAudioConfig);
    }

    return ConnectVisemeSignal() && ConnectSynthesisStartedSignal() && ConnectSynthesisUpdateSignals();
}

bool FAzSpeechSynthesisRunnable::ConnectVisemeSignal()
{
    UAzSpeechSynthesizerTaskBase* const SynthesizerTask = GetOwningSynthesizerTask();
    if (!IsSpeechSynthesizerValid() || !UAzSpeechTaskStatus::IsTaskStillValid(SynthesizerTask))
    {
        return false;
    }

    if (!SynthesizerTask->GetSynthesisOptions().bEnableViseme)
    {
        return true;
    }

    bFilterVisemeData = SynthesizerTask->bIsSSMLBased && UAzSpeechSettings::Get()->bFilterVisemeFacialExpression && SynthesizerTask->SynthesisText.Contains("<mstts:viseme type=\"FacialExpression\"/>", ESearchCase::IgnoreCase);

    SpeechSynthesizer->VisemeReceived.Connect(
        [this, SynthesizerTask](const MicrosoftSpeech::SpeechSynthesisVisemeEventArgs& VisemeEventArgs)
        {
            if (!UAzSpeechTaskStatus::IsTaskStillValid(SynthesizerTask))
            {
                StopAzSpeechRunnableTask();
                return;
            }

            if (bFilterVisemeData && VisemeEventArgs.Animation.empty())
            {
                return;
            }

            FAzSpeechVisemeData LastVisemeData;
            LastVisemeData.VisemeID = VisemeEventArgs.VisemeId;
            LastVisemeData.AudioOffsetMilliseconds = VisemeEventArgs.AudioOffset / 10000;
            LastVisemeData.Animation = FString(UTF8_TO_TCHAR(VisemeEventArgs.Animation.c_str()));

            AsyncTask(ENamedThreads::GameThread,
                [SynthesizerTask, LastVisemeData]
                {
                    SynthesizerTask->OnVisemeReceived(LastVisemeData);
                }
            );
        }
    );

    return true;
}

bool FAzSpeechSynthesisRunnable::ConnectSynthesisStartedSignal()
{
    UAzSpeechSynthesizerTaskBase* const SynthesizerTask = GetOwningSynthesizerTask();
    if (!IsSpeechSynthesizerValid() || !UAzSpeechTaskStatus::IsTaskStillValid(SynthesizerTask))
    {
        return false;
    }

    SpeechSynthesizer->SynthesisStarted.Connect(
        [this, SynthesizerTask]([[maybe_unused]] const MicrosoftSpeech::SpeechSynthesisEventArgs& SynthesisEventArgs)
        {
            if (!UAzSpeechTaskStatus::IsTaskStillValid(SynthesizerTask))
            {
                StopAzSpeechRunnableTask();
            }
            else
            {
                AsyncTask(ENamedThreads::GameThread,
                    [SynthesizerTask]
                    {
                        SynthesizerTask->SynthesisStarted.Broadcast();
                    }
                );
            }
        }
    );

    return true;
}

bool FAzSpeechSynthesisRunnable::ConnectSynthesisUpdateSignals()
{
    UAzSpeechSynthesizerTaskBase* const SynthesizerTask = GetOwningSynthesizerTask();
    if (!IsSpeechSynthesizerValid() || !UAzSpeechTaskStatus::IsTaskStillValid(SynthesizerTask))
    {
        return false;
    }

    SpeechSynthesizer->Synthesizing.Connect(
        [this, SynthesizerTask](const MicrosoftSpeech::SpeechSynthesisEventArgs& SynthesisEventArgs)
        {
            if (!UAzSpeechTaskStatus::IsTaskStillValid(SynthesizerTask))
            {
                StopAzSpeechRunnableTask();
            }
            else
            {
                AsyncTask(ENamedThreads::GameThread,
                    [SynthesizerTask, Result = SynthesisEventArgs.Result]
                    {
                        SynthesizerTask->OnSynthesisUpdate(Result);
                    }
                );
            }
        }
    );

    const auto TaskResultReach_Lambda = [this, SynthesizerTask](const MicrosoftSpeech::SpeechSynthesisEventArgs& SynthesisEventArgs)
        {
            if (!UAzSpeechTaskStatus::IsTaskStillValid(SynthesizerTask))
            {
                StopAzSpeechRunnableTask();
                return;
            }

            const bool bValidResult = ProcessSynthesisResult(SynthesisEventArgs.Result);
            if (!bValidResult)
            {
                AsyncTask(ENamedThreads::GameThread,
                    [SynthesizerTask]
                    {
                        SynthesizerTask->SynthesisFailed.Broadcast();
                    }
                );
            }
            else
            {
                AsyncTask(ENamedThreads::GameThread,
                    [SynthesizerTask, Result = SynthesisEventArgs.Result]
                    {
                        SynthesizerTask->OnSynthesisUpdate(Result);
                        SynthesizerTask->BroadcastFinalResult();
                    }
                );
            }

            StopAzSpeechRunnableTask();
        };

    SpeechSynthesizer->SynthesisCanceled.Connect(TaskResultReach_Lambda);
    SpeechSynthesizer->SynthesisCompleted.Connect(TaskResultReach_Lambda);

    return true;
}

bool FAzSpeechSynthesisRunnable::ProcessSynthesisResult(const std::shared_ptr<MicrosoftSpeech::SpeechSynthesisResult>& LastResult)
{
    bool bOutput = true;

    switch (LastResult->Reason)
    {
    case MicrosoftSpeech::ResultReason::SynthesizingAudio:
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Task running. Reason: SynthesizingAudio"), *GetThreadName(), *FString(__func__));
        break;

    case MicrosoftSpeech::ResultReason::SynthesizingAudioCompleted:
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Task completed. Reason: SynthesizingAudioCompleted"), *GetThreadName(), *FString(__func__));
        break;

    case MicrosoftSpeech::ResultReason::SynthesizingAudioStarted:
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("Thread: %s; Function: %s; Message: Task started. Reason: SynthesizingAudioStarted"), *GetThreadName(), *FString(__func__));
        break;

    default:
        break;
    }

    if (LastResult->Reason == MicrosoftSpeech::ResultReason::Canceled)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Task failed. Reason: Canceled"), *GetThreadName(), *FString(__func__));

        bOutput = false;
        const auto CancellationDetails = MicrosoftSpeech::SpeechSynthesisCancellationDetails::FromResult(LastResult);

        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Cancellation Reason: %s"), *GetThreadName(), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));
        if (CancellationDetails->Reason == MicrosoftSpeech::CancellationReason::Error)
        {
            ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
        }
    }

    return bOutput;
}

const MicrosoftSpeech::SpeechSynthesisOutputFormat FAzSpeechSynthesisRunnable::GetOutputFormat() const
{
    if (UAzSpeechSynthesizerTaskBase* const SynthesizerTask = GetOwningSynthesizerTask(); UAzSpeechTaskStatus::IsTaskStillValid(SynthesizerTask))
    {
        switch (SynthesizerTask->GetSynthesisOptions().SpeechSynthesisOutputFormat)
        {
        case EAzSpeechSynthesisOutputFormat::Riff16Khz16BitMonoPcm:
            return MicrosoftSpeech::SpeechSynthesisOutputFormat::Riff16Khz16BitMonoPcm;

        case EAzSpeechSynthesisOutputFormat::Riff24Khz16BitMonoPcm:
            return MicrosoftSpeech::SpeechSynthesisOutputFormat::Riff24Khz16BitMonoPcm;

        case EAzSpeechSynthesisOutputFormat::Riff48Khz16BitMonoPcm:
            return MicrosoftSpeech::SpeechSynthesisOutputFormat::Riff48Khz16BitMonoPcm;

        case EAzSpeechSynthesisOutputFormat::Riff22050Hz16BitMonoPcm:
            return MicrosoftSpeech::SpeechSynthesisOutputFormat::Riff22050Hz16BitMonoPcm;

        case EAzSpeechSynthesisOutputFormat::Riff44100Hz16BitMonoPcm:
            return MicrosoftSpeech::SpeechSynthesisOutputFormat::Riff44100Hz16BitMonoPcm;

        default:
            break;
        }
    }

    return MicrosoftSpeech::SpeechSynthesisOutputFormat::Riff16Khz16BitMonoPcm;
}