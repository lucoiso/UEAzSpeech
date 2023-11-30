// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/Recognition/AzSpeechRecognitionRunnable.h"
#include "AzSpeech/Tasks/Recognition/Bases/AzSpeechRecognizerTaskBase.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>
#include <Misc/ScopeTryLock.h>

namespace MicrosoftSpeech = Microsoft::CognitiveServices::Speech;

FAzSpeechRecognitionRunnable::FAzSpeechRecognitionRunnable(UAzSpeechTaskBase* const InOwningTask, const std::shared_ptr<MicrosoftSpeech::Audio::AudioConfig>& InAudioConfig)
    : FAzSpeechRecognitionRunnableBase(InOwningTask, InAudioConfig)
{
}

uint32 FAzSpeechRecognitionRunnable::Run()
{
    if (FAzSpeechRecognitionRunnableBase::Run() == 0u)
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

    const float SleepTime = GetThreadUpdateInterval();
    while (!IsPendingStop())
    {
        FPlatformProcess::Sleep(SleepTime);
    }

    return 1u;
}