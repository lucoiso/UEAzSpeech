// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechEngineSubsystem.h"
#include "AzSpeech/Tasks/Synthesis/Bases/AzSpeechSpeechSynthesisBase.h"
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
    UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Inserting task %s (%d) in AzSpeech Engine Subsystem execution queue with ID '%d'."), *FString(__func__), *Task->TaskName.ToString(), Task->GetUniqueID(), QueueId);

    const bool bTaskStatus = TaskQueueMap.Contains(QueueId) ? TaskQueueMap.FindRef(QueueId).Key : false;
    AzSpeechTaskQueueValue& Item = TaskQueueMap.FindOrAdd(QueueId);
    Item.Key = bTaskStatus;
    Item.Value.Emplace(Task);
}

void UAzSpeechEngineSubsystem::InvalidateQueue(const int64 QueueId) const
{
    UE_LOG(LogAzSpeech, Display, TEXT("%s: Invalidating AzSpeech Engine Subsystem execution queue with ID '%d'."), *FString(__func__), QueueId);

    if (TaskQueueMap.Contains(QueueId) || TaskAudioQueueMap.Contains(QueueId))
    {
        TaskQueueMap.Remove(QueueId);
        TaskAudioQueueMap.Remove(QueueId);
    }
}

void UAzSpeechEngineSubsystem::InitializeQueueExecution(const int64 QueueId) const
{
    if (TaskQueueMap.Contains(QueueId))
    {
        if (TaskQueueMap.Find(QueueId)->Key)
        {
            return;
        }

        UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Initializing AzSpeech Engine Subsystem execution queue with ID '%d'."), *FString(__func__), QueueId);

        TaskQueueMap.Find(QueueId)->Key = true;
        DequeueExecutionQueue(QueueId);
    }
    else
    {
        InvalidateQueue(QueueId);
    }
}

void UAzSpeechEngineSubsystem::StopQueueExecution(const int64 QueueId) const
{
    if (TaskQueueMap.Contains(QueueId))
    {
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Stopping AzSpeech Engine Subsystem execution queue with ID '%d'."), *FString(__func__), QueueId);

        TaskQueueMap.Find(QueueId)->Key = false;
    }
    else
    {
        InvalidateQueue(QueueId);
    }
}

bool UAzSpeechEngineSubsystem::IsQueueActive(const int64 QueueId) const
{
    if (!TaskQueueMap.Contains(QueueId))
    {
        return false;
    }

    return TaskQueueMap.FindRef(QueueId).Key;
}

bool UAzSpeechEngineSubsystem::IsQueueEmpty(const int64 QueueId) const
{
    if (!TaskQueueMap.Contains(QueueId))
    {
        return true;
    }

    return TaskQueueMap.Find(QueueId)->Value.IsEmpty();
}

void UAzSpeechEngineSubsystem::RegisterAzSpeechTask(UAzSpeechTaskBase* const Task) const
{
    if (UAzSpeechTaskStatus::IsTaskStillValid(Task) && !RegisteredTasks.Contains(Task))
    {
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Registering task %s (%d) in AzSpeech Engine Subsystem."), *FString(__func__), *Task->TaskName.ToString(), Task->GetUniqueID());

        RegisteredTasks.Add(Task);
        OnAzSpeechTaskRegistered.Broadcast(FAzSpeechTaskData{ static_cast<int64>(Task->GetUniqueID()), Task->GetClass() });
    }

    ValidateRegisteredTasks();
}

void UAzSpeechEngineSubsystem::UnregisterAzSpeechTask(UAzSpeechTaskBase* const Task) const
{
    if (UAzSpeechTaskStatus::IsTaskStillValid(Task) && RegisteredTasks.Contains(Task))
    {
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Unregistering task %s (%d) from AzSpeech Engine Subsystem."), *FString(__func__), *Task->TaskName.ToString(), Task->GetUniqueID());

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
    if (!ExecQueue || ExecQueue->Value.IsEmpty())
    {
        TaskQueueMap.Remove(QueueId);
        return;
    }

    if (!ExecQueue->Key)
    {
        return;
    }

    UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Dequeuing AzSpeech Engine Subsystem execution queue with ID '%d'."), *FString(__func__), QueueId);

    const TWeakObjectPtr<UAzSpeechTaskBase> Task = ExecQueue->Value[0];

    if (Task.IsValid())
    {
        Task->InternalOnTaskFinished.BindUObject(this, &UAzSpeechEngineSubsystem::OnQueueExecutionCompleted, QueueId);

        if (UAzSpeechSpeechSynthesisBase* const SpeechSynthesis = Cast<UAzSpeechSpeechSynthesisBase>(Task))
        {
            SpeechSynthesis->bAutoPlayAudio = false;
            SpeechSynthesis->InternalAudioFinished.BindUObject(this, &UAzSpeechEngineSubsystem::OnQueueAudioExecutionCompleted, QueueId);

            TaskAudioQueueMap.FindOrAdd(QueueId).Emplace(SpeechSynthesis);
        }

        Task->Activate();
    }
    else
    {
        DequeueExecutionQueue(QueueId);
    }
}

void UAzSpeechEngineSubsystem::OnQueueExecutionCompleted(const FAzSpeechTaskData Data, const int64 QueueId) const
{
    if (Data.Class && Data.Class->IsChildOf<UAzSpeechSpeechSynthesisBase>() && TaskAudioQueueMap.Contains(QueueId))
    {
        if (const TArray<TWeakObjectPtr<class UAzSpeechTaskBase>>& TaskQueue = TaskAudioQueueMap.FindRef(QueueId);
            TaskQueue.Num() == 1 && TaskQueue[0].IsValid() && static_cast<int64>(TaskQueue[0]->GetUniqueID()) == Data.UniqueID)
        {
            DequeueAudioExecutionQueue(QueueId);
        }
    }

    OnAzSpeechExecutionQueueProgressed.Broadcast(QueueId, Data);

    AzSpeechTaskQueueValue* const ExecQueue = TaskQueueMap.Find(QueueId);
    if (!ExecQueue || ExecQueue->Value.IsEmpty())
    {
        TaskQueueMap.Remove(QueueId);
        return;
    }

    ExecQueue->Value.RemoveAt(0);

    if (ExecQueue->Value.IsEmpty())
    {
        TaskQueueMap.Remove(QueueId);

        if (!TaskAudioQueueMap.Contains(QueueId) || TaskAudioQueueMap.FindRef(QueueId).IsEmpty())
        {
            InvalidateQueue(QueueId);
        }
    }
    else
    {
        DequeueExecutionQueue(QueueId);
    }
}

void UAzSpeechEngineSubsystem::DequeueAudioExecutionQueue(const int64 QueueId) const
{
    if (!TaskAudioQueueMap.Contains(QueueId))
    {
        return;
    }

    TArray<TWeakObjectPtr<class UAzSpeechTaskBase>>* const ExecQueue = TaskAudioQueueMap.Find(QueueId);
    if (!ExecQueue || ExecQueue->IsEmpty())
    {
        TaskAudioQueueMap.Remove(QueueId);
        return;
    }

    UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Dequeuing AzSpeech Engine Subsystem audio execution queue with ID '%d'."), *FString(__func__), QueueId);

    const TWeakObjectPtr<UAzSpeechTaskBase> Task = (*ExecQueue)[0];

    if (Task.IsValid() && Task->IsA<UAzSpeechSpeechSynthesisBase>())
    {
        Cast<UAzSpeechSpeechSynthesisBase>(Task)->PlayAudio();
    }
    else
    {
        DequeueAudioExecutionQueue(QueueId);
    }
}

void UAzSpeechEngineSubsystem::OnQueueAudioExecutionCompleted([[maybe_unused]] const FAzSpeechTaskData Data, const int64 QueueId) const
{
    TArray<TWeakObjectPtr<class UAzSpeechTaskBase>>* const ExecQueue = TaskAudioQueueMap.Find(QueueId);
    if (!ExecQueue)
    {
        return;
    }

    if (!ExecQueue || ExecQueue->IsEmpty())
    {
        TaskAudioQueueMap.Remove(QueueId);
        return;
    }

    ExecQueue->RemoveAt(0);

    if (ExecQueue->IsEmpty())
    {
        TaskAudioQueueMap.Remove(QueueId);

        if (!TaskQueueMap.Contains(QueueId) || TaskQueueMap.FindRef(QueueId).Value.IsEmpty())
        {
            InvalidateQueue(QueueId);
        }
    }
    else
    {
        DequeueAudioExecutionQueue(QueueId);
    }
}