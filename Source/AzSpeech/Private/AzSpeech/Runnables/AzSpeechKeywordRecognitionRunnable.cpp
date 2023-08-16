// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/AzSpeechKeywordRecognitionRunnable.h"
#include "AzSpeech/Tasks/Bases/AzSpeechRecognizerTaskBase.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>
#include <Misc/ScopeTryLock.h>

FAzSpeechKeywordRecognitionRunnable::FAzSpeechKeywordRecognitionRunnable(UAzSpeechTaskBase* const InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig, const std::shared_ptr<Microsoft::CognitiveServices::Speech::KeywordRecognitionModel>& InModel) : FAzSpeechRecognitionRunnableBase(InOwningTask, InAudioConfig), Model(InModel)
{
}

uint32 FAzSpeechKeywordRecognitionRunnable::Run()
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

    if (!Model)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Thread: %s; Function: %s; Message: Model is invalid"), *GetThreadName(), *FString(__func__));
        return 0u;
    }

    UAzSpeechRecognizerTaskBase* const RecognizerTask = GetOwningRecognizerTask();
    if (!UAzSpeechTaskStatus::IsTaskStillValid(RecognizerTask))
    {
        return 0u;
    }

    const std::future<void> Future = SpeechRecognizer->StartKeywordRecognitionAsync(Model);

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

void FAzSpeechKeywordRecognitionRunnable::Exit()
{
    FScopeTryLock Lock(&Mutex);

    FAzSpeechRecognitionRunnableBase::Exit();

    if (Lock.IsLocked() && SpeechRecognizer)
    {
        SpeechRecognizer->StopKeywordRecognitionAsync().wait_for(GetTaskTimeout());
    }

    SpeechRecognizer = nullptr;
}