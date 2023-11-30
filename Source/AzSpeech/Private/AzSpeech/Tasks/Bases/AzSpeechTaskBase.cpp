// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechTaskBase.h"
#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeech/AzSpeechEngineSubsystem.h"
#include "AzSpeech/Structures/AzSpeechTaskData.h"
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"
#include <Misc/ScopeTryLock.h>

#if WITH_EDITOR
#include <Editor.h>
#endif

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechTaskBase)
#endif

void UAzSpeechTaskBase::Activate()
{
#if PLATFORM_ANDROID
    if (!UAzSpeechHelper::CheckAndroidPermission("android.permission.INTERNET"))
    {
        SetReadyToDestroy();
        return;
    }
#endif

    FString NewTaskName = TaskName.ToString();
    NewTaskName.RemoveFromEnd("_DefaultOptions");
    NewTaskName.RemoveFromEnd("_CustomOptions");

    TaskName = *NewTaskName;

    SubscriptionOptions.SyncEndpointWithRegion();

    UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Activating task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

    bIsTaskActive = true;

    Super::Activate();

    if (!StartAzureTaskWork())
    {
        UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%d); Function: %s; Message: Failed to activate task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
        SetReadyToDestroy();

        return;
    }

    if (const UAzSpeechEngineSubsystem* const Subsystem = GEngine->GetEngineSubsystem<UAzSpeechEngineSubsystem>())
    {
        Subsystem->RegisterAzSpeechTask(this);
    }

#if WITH_EDITOR
    if (bIsEditorTask)
    {
        SetFlags(RF_Standalone);
    }
    else
    {
        FEditorDelegates::PrePIEEnded.AddUObject(this, &UAzSpeechTaskBase::PrePIEEnded);
    }
#endif
}

void UAzSpeechTaskBase::StopAzSpeechTask()
{
    FScopeTryLock TryLock(&Mutex);

    if (!TryLock.IsLocked() || !UAzSpeechTaskStatus::IsTaskActive(this) || UAzSpeechTaskStatus::IsTaskReadyToDestroy(this))
    {
        return;
    }

    UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Stopping task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
    bIsTaskActive = false;
    
    if (RunnableTask.IsValid())
    {
        RunnableTask->StopAzSpeechRunnableTask();
    }

    SetReadyToDestroy();
}

const FName UAzSpeechTaskBase::GetTaskName() const
{
    return TaskName;
}

const FAzSpeechSubscriptionOptions& UAzSpeechTaskBase::GetSubscriptionOptions() const
{
    return SubscriptionOptions;
}

void UAzSpeechTaskBase::SetSubscriptionOptions(const FAzSpeechSubscriptionOptions& Options)
{
    if (UAzSpeechTaskStatus::IsTaskActive(this))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Can't change the options while the task is active."), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
        return;
    }

    SubscriptionOptions = Options;
}

void UAzSpeechTaskBase::SetReadyToDestroy()
{
    FScopeTryLock TryLock(&Mutex);

    if (!TryLock.IsLocked() || UAzSpeechTaskStatus::IsTaskReadyToDestroy(this))
    {
        return;
    }

    InternalOnTaskFinished.ExecuteIfBound(FAzSpeechTaskData{ GetUniqueID(), GetClass() });
    InternalOnTaskFinished.Unbind();

    if (const UAzSpeechEngineSubsystem* const Subsystem = GEngine->GetEngineSubsystem<UAzSpeechEngineSubsystem>())
    {
        Subsystem->UnregisterAzSpeechTask(this);
    }

    UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Setting task as Ready to Destroy"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
    bIsReadyToDestroy = true;

#if WITH_EDITOR
    if (bIsEditorTask)
    {
        ClearFlags(RF_Standalone);

#if ENGINE_MAJOR_VERSION >= 5
        MarkAsGarbage();
#else
        MarkPendingKill();
#endif
    }

    if (FEditorDelegates::PrePIEEnded.IsBoundToObject(this))
    {
        FEditorDelegates::PrePIEEnded.RemoveAll(this);
    }
#endif

    if (RunnableTask.IsValid())
    {
        RunnableTask->StopAzSpeechRunnableTask();
    }

    Super::SetReadyToDestroy();
}

bool UAzSpeechTaskBase::StartAzureTaskWork()
{
    UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Starting Azure SDK task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

    return UAzSpeechTaskStatus::IsTaskStillValid(this);
}

void UAzSpeechTaskBase::BroadcastFinalResult()
{
    check(IsInGameThread());

    FScopeLock Lock(&Mutex);

    if (!bIsTaskActive)
    {
        return;
    }

    InternalOnTaskFinished.ExecuteIfBound(FAzSpeechTaskData{ GetUniqueID(), GetClass() });
    InternalOnTaskFinished.Unbind();

    UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Task completed, broadcasting final result"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

    bIsTaskActive = false;
}

#if WITH_EDITOR
void UAzSpeechTaskBase::PrePIEEnded(bool bIsSimulating)
{
    if (!UAzSpeechTaskStatus::IsTaskStillValid(this))
    {
        return;
    }

    UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Trying to finish task due to PIE end"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

    bEndingPIE = true;
    StopAzSpeechTask();
}
#endif

bool UAzSpeechTaskStatus::IsTaskActive(const UAzSpeechTaskBase* const Test)
{
    return IsValid(Test) && Test->bIsTaskActive;
}

bool UAzSpeechTaskStatus::IsTaskReadyToDestroy(const UAzSpeechTaskBase* const Test)
{
    return IsValid(Test) && Test->bIsReadyToDestroy;
}

bool UAzSpeechTaskStatus::IsTaskStillValid(const UAzSpeechTaskBase* const Test)
{
    bool bOutput = IsValid(Test) && !IsTaskReadyToDestroy(Test);

#if WITH_EDITOR
    bOutput = bOutput && !Test->bEndingPIE;
#endif

    return bOutput;
}