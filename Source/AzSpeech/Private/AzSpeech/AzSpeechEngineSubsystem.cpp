// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechEngineSubsystem.h"
#include "AzSpeech/Tasks/Bases/AzSpeechTaskBase.h"
#include "LogAzSpeech.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechEngineSubsystem)
#endif

UAzSpeechEngineSubsystem::UAzSpeechEngineSubsystem()
    : UEngineSubsystem()
    , RegisteredTasks()
{
}

void UAzSpeechEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Engine Subsystem initialized."), *FString(__func__));
}

void UAzSpeechEngineSubsystem::Deinitialize()
{
    UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Engine Subsystem deinitialized."), *FString(__func__));

    Super::Deinitialize();
}

UAzSpeechTaskBase* UAzSpeechEngineSubsystem::GetRegisteredAzSpeechTask(const FAzSpeechTaskData& Data) const
{
    ValidateRegisteredTasks();

    const TWeakObjectPtr<UAzSpeechTaskBase>* const MatchingTask = RegisteredTasks.FindByPredicate(
        [Data](const TWeakObjectPtr<UAzSpeechTaskBase>& Task)
        {
            return Data.UniqueID == Task->GetUniqueID() && Task->IsA(Data.Class);
        }
    );

    return MatchingTask ? MatchingTask->Get() : nullptr;
}

TArray<class UAzSpeechTaskBase*> UAzSpeechEngineSubsystem::GetAllRegisteredAzSpeechTasks() const
{
    ValidateRegisteredTasks();

    TArray<class UAzSpeechTaskBase*> Output;

    for (const TWeakObjectPtr<UAzSpeechTaskBase>& TaskIt : RegisteredTasks)
    {
        Output.Add(TaskIt.Get());
    }

    return Output;
}

void UAzSpeechEngineSubsystem::InsertTaskInQueue(const int64 QueueId, UAzSpeechTaskBase* const Task) const
{
    UE_LOG(LogAzSpeech, Display, TEXT("%s: Inserting task %s (%d) in AzSpeech Engine Subsystem execution queue with ID '%d'."), *FString(__func__), *Task->TaskName.ToString(), Task->GetUniqueID(), QueueId);

    const bool bTaskStatus = TaskQueueMap.Contains(QueueId) ? TaskQueueMap.FindRef(QueueId).Key : false;
    AzSpeechTaskQueueValue& Item = TaskQueueMap.FindOrAdd(QueueId);
    Item.Key = bTaskStatus;
    Item.Value = { TWeakObjectPtr<UAzSpeechTaskBase>(Task) };
}

void UAzSpeechEngineSubsystem::ResetQueue(const int64 QueueId) const
{
    UE_LOG(LogAzSpeech, Display, TEXT("%s: Resetting AzSpeech Engine Subsystem execution queue with ID '%d'."), *FString(__func__), QueueId);
    TaskQueueMap.Remove(QueueId);
}

void UAzSpeechEngineSubsystem::InitializeQueueExecution(const int64 QueueId) const
{
    if (TaskQueueMap.Contains(QueueId))
    {
        UE_LOG(LogAzSpeech, Display, TEXT("%s: Initializing AzSpeech Engine Subsystem execution queue with ID '%d'."), *FString(__func__), QueueId);

        TaskQueueMap.Find(QueueId)->Key = true;
    }
    else
    {
        ResetQueue(QueueId);
    }
}

void UAzSpeechEngineSubsystem::StopQueueExecution(const int64 QueueId) const
{
    if (TaskQueueMap.Contains(QueueId))
    {
        UE_LOG(LogAzSpeech, Display, TEXT("%s: Stopping AzSpeech Engine Subsystem execution queue with ID '%d'."), *FString(__func__), QueueId);

        TaskQueueMap.Find(QueueId)->Key = false;
    }
    else
    {
        ResetQueue(QueueId);
    }
}

bool UAzSpeechEngineSubsystem::IsQueueActive(const int64 QueueId) const
{
    if (!TaskQueueMap.Contains(QueueId))
    {
        ResetQueue(QueueId);
        return false;
    }

    return TaskQueueMap.FindRef(QueueId).Key;
}

bool UAzSpeechEngineSubsystem::IsQueueEmpty(const int64 QueueId) const
{
    if (!TaskQueueMap.Contains(QueueId))
    {
        ResetQueue(QueueId);
        return true;
    }

    return TaskQueueMap.Find(QueueId)->Value.IsEmpty();
}

void UAzSpeechEngineSubsystem::RegisterAzSpeechTask(UAzSpeechTaskBase* const Task) const
{
    if (UAzSpeechTaskStatus::IsTaskStillValid(Task) && !RegisteredTasks.Contains(Task))
    {
        UE_LOG(LogAzSpeech, Display, TEXT("%s: Registering task %s (%d) in AzSpeech Engine Subsystem."), *FString(__func__), *Task->TaskName.ToString(), Task->GetUniqueID());

        RegisteredTasks.Add(Task);
        OnAzSpeechTaskRegistered.Broadcast(FAzSpeechTaskData{ static_cast<int64>(Task->GetUniqueID()), Task->GetClass() });
    }

    ValidateRegisteredTasks();
}

void UAzSpeechEngineSubsystem::UnregisterAzSpeechTask(UAzSpeechTaskBase* const Task) const
{
    if (UAzSpeechTaskStatus::IsTaskStillValid(Task) && RegisteredTasks.Contains(Task))
    {
        UE_LOG(LogAzSpeech, Display, TEXT("%s: Unregistering task %s (%d) from AzSpeech Engine Subsystem."), *FString(__func__), *Task->TaskName.ToString(), Task->GetUniqueID());

        RegisteredTasks.Remove(Task);
        OnAzSpeechTaskUnregistered.Broadcast(FAzSpeechTaskData{ static_cast<int64>(Task->GetUniqueID()), Task->GetClass() });
    }

    ValidateRegisteredTasks();
}

void UAzSpeechEngineSubsystem::ValidateRegisteredTasks() const
{
    RegisteredTasks.RemoveAll(
        [this](const TWeakObjectPtr<UAzSpeechTaskBase>& Task)
        {
            return !UAzSpeechTaskStatus::IsTaskStillValid(Task.Get());
        }
    );
}

void UAzSpeechEngineSubsystem::DequeueExecutionQueue(const int64 QueueId) const
{
    if (!TaskQueueMap.Contains(QueueId))
    {
        return;
    }

    AzSpeechTaskQueueValue* const ExecQueue = TaskQueueMap.Find(QueueId);
    if (!ExecQueue || !ExecQueue->Key)
    {
        return;
    }

    if (ExecQueue->Value.IsEmpty())
    {
        ResetQueue(QueueId);
        return;
    }

    UE_LOG(LogAzSpeech, Display, TEXT("%s: Dequeuing AzSpeech Engine Subsystem execution queue with ID '%d'."), *FString(__func__), QueueId);

    const TWeakObjectPtr<UAzSpeechTaskBase> Task = ExecQueue->Value[0];
    ExecQueue->Value.RemoveAt(0);

    if (Task.IsValid())
    {
        Task->InternalOnTaskFinished.BindUObject(this, &UAzSpeechEngineSubsystem::OnQueueExecutionCompleted, QueueId);
        Task->Activate();
    }
    else
    {
        DequeueExecutionQueue(QueueId);
    }
}

void UAzSpeechEngineSubsystem::OnQueueExecutionCompleted(const FAzSpeechTaskData Data, const int64 QueueId) const
{
    OnAzSpeechExecutionQueueProgressed.Broadcast(QueueId, Data);
    DequeueExecutionQueue(QueueId);
}