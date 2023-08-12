// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Subsystems/EngineSubsystem.h>
#include "AzSpeech/Structures/AzSpeechTaskData.h"
#include "AzSpeechEngineSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAzSpeechTaskRegistrationUpdate, const FAzSpeechTaskData, TaskData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAzSpeechTaskQueueExecutionProgress, const int64, QueueId, const FAzSpeechTaskData, TaskData);

UCLASS()
class UAzSpeechEngineSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

    friend class UAzSpeechTaskBase;

public:
    explicit UAzSpeechEngineSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    UAzSpeechTaskBase* GetRegisteredAzSpeechTask(const FAzSpeechTaskData& Data) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    TArray<class UAzSpeechTaskBase*> GetAllRegisteredAzSpeechTasks() const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    void InsertTaskInQueue(const int64 QueueId, UAzSpeechTaskBase* const Task) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    void ResetQueue(const int64 QueueId) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    void InitializeQueueExecution(const int64 QueueId) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    void StopQueueExecution(const int64 QueueId) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    bool IsQueueActive(const int64 QueueId) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    bool IsQueueEmpty(const int64 QueueId) const;

private:
    void RegisterAzSpeechTask(class UAzSpeechTaskBase* const Task) const;
    void UnregisterAzSpeechTask(class UAzSpeechTaskBase* const Task) const;

    void ValidateRegisteredTasks() const;

    void DequeueExecutionQueue(const int64 QueueId) const;
    void OnQueueExecutionCompleted(const FAzSpeechTaskData Data, const int64 QueueId) const;

public:
    UPROPERTY(BlueprintAssignable)
    FAzSpeechTaskRegistrationUpdate OnAzSpeechTaskRegistered;

    UPROPERTY(BlueprintAssignable)
    FAzSpeechTaskRegistrationUpdate OnAzSpeechTaskUnregistered;

    UPROPERTY(BlueprintAssignable)
    FAzSpeechTaskQueueExecutionProgress OnAzSpeechExecutionQueueProgressed;

private:
    mutable TArray<TWeakObjectPtr<class UAzSpeechTaskBase>> RegisteredTasks;

    typedef TPair<bool, TArray<TWeakObjectPtr<class UAzSpeechTaskBase>>> AzSpeechTaskQueueValue;
    mutable TMap<int64, AzSpeechTaskQueueValue> TaskQueueMap;
};