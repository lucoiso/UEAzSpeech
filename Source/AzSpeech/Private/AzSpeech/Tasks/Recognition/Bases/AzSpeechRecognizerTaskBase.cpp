// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Recognition/Bases/AzSpeechRecognizerTaskBase.h"
#include "AzSpeech/Runnables/Recognition/AzSpeechRecognitionRunnable.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>

#if !UE_BUILD_SHIPPING
#include <Engine/Engine.h>
#endif

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechRecognizerTaskBase)
#endif

namespace MicrosoftSpeech = Microsoft::CognitiveServices::Speech;

const FAzSpeechRecognitionOptions& UAzSpeechRecognizerTaskBase::GetRecognitionOptions() const
{
    return RecognitionOptions;
}

void UAzSpeechRecognizerTaskBase::SetRecognitionOptions(const FAzSpeechRecognitionOptions& Options)
{
    if (UAzSpeechTaskStatus::IsTaskActive(this))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Can't change the options while the task is active."), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
        return;
    }

    RecognitionOptions = Options;
}

const FString UAzSpeechRecognizerTaskBase::GetRecognizedString() const
{
    FScopeLock Lock(&Mutex);

    if (RecognizedText.empty())
    {
        return FString();
    }

    return FString(UTF8_TO_TCHAR(RecognizedText.c_str()));
}

const int64 UAzSpeechRecognizerTaskBase::GetRecognitionDuration() const
{
    FScopeLock Lock(&Mutex);

    return RecognitionDuration;
}

const int32 UAzSpeechRecognizerTaskBase::GetRecognitionLatency() const
{
    FScopeLock Lock(&Mutex);

    return RecognitionLatency;
}

void UAzSpeechRecognizerTaskBase::StartRecognitionWork()
{
    RunnableTask = MakeUnique<FAzSpeechRecognitionRunnable>(this, AudioConfig);

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

    AsyncTask(ENamedThreads::GameThread,
        [this]
        {
            RecognitionCompleted.Broadcast(GetRecognizedString());
            SetReadyToDestroy();
        }
    );
}

void UAzSpeechRecognizerTaskBase::OnRecognitionUpdated(const std::shared_ptr<MicrosoftSpeech::SpeechRecognitionResult>& LastResult)
{
    check(IsInGameThread());

    FScopeLock Lock(&Mutex);

    RecognizedText = LastResult->Text;

    const auto TicksToMs = [](const auto& Ticks)
        {
            return static_cast<int64>(Ticks / 10000u);
        };

    RecognitionDuration = TicksToMs(LastResult->Duration());
    RecognitionLatency = GetProperty<int32>(LastResult, MicrosoftSpeech::PropertyId::SpeechServiceResponse_RecognitionLatencyMs);

    RecognitionUpdated.Broadcast(GetRecognizedString());

    if (UAzSpeechSettings::Get()->bEnableDebuggingLogs || UAzSpeechSettings::Get()->bEnableDebuggingPrints)
    {
        const FStringFormatOrderedArguments Arguments{
            TaskName.ToString(),
            GetUniqueID(),
            FString(__func__),
            FString(UTF8_TO_TCHAR(LastResult->Text.c_str())),
            RecognitionDuration,
            TicksToMs(LastResult->Offset()),
            static_cast<int32>(LastResult->Reason),
            FString(UTF8_TO_TCHAR(LastResult->ResultId.c_str())),
            RecognitionLatency
        };

        const FString MountedDebuggingInfo = FString::Format(TEXT("Task: {0} ({1});\n\tFunction: {2};\n\tRecognized text: {3}\n\tDuration: {4}ms\n\tOffset: {5}ms\n\tReason code: {6}\n\tResult ID: {7}\n\tRecognition latency: {8}ms"), Arguments);

        UE_LOG(LogAzSpeech_Debugging, Display, TEXT("%s"), *MountedDebuggingInfo);

#if !UE_BUILD_SHIPPING
        if (UAzSpeechSettings::Get()->bEnableDebuggingPrints)
        {
            GEngine->AddOnScreenDebugMessage(static_cast<int32>(GetUniqueID()), 5.f, FColor::Yellow, MountedDebuggingInfo);
        }
#endif
    }
}