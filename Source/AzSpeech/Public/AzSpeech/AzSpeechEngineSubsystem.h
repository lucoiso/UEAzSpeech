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

UCLASS(Category = "AzSpeech")
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
    void InsertTaskInQueue(const int64 QueueId, class UAzSpeechTaskBase* const Task) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    void InvalidateQueue(const int64 QueueId) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    void InitializeQueueExecution(const int64 QueueId) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    void StopQueueExecution(const int64 QueueId) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    bool IsQueueActive(const int64 QueueId) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Management")
    bool IsQueueEmpty(const int64 QueueId) const;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Tasks", meta = (WorldContext = "WorldContextObject"))
    class UAzSpeechTaskBase* CreateSpeechToTextTask(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions SubscriptionOptions, const FAzSpeechRecognitionOptions RecognitionOptions, const FString& AudioInputDeviceID = "Default", const FName PhraseListGroup = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Tasks", meta = (WorldContext = "WorldContextObject"))
    class UAzSpeechTaskBase* CreateSSMLToAudioDataTask(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions SubscriptionOptions, const FAzSpeechSynthesisOptions SynthesisOptions, const FString& SynthesisSSML);

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Tasks", meta = (WorldContext = "WorldContextObject"))
    class UAzSpeechTaskBase* CreateSSMLToSoundWaveTask(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions SubscriptionOptions, const FAzSpeechSynthesisOptions SynthesisOptions, const FString& SynthesisSSML);

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Tasks", meta = (WorldContext = "WorldContextObject"))
    class UAzSpeechTaskBase* CreateSSMLToSpeechTask(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions SubscriptionOptions, const FAzSpeechSynthesisOptions SynthesisOptions, const FString& SynthesisSSML);

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Tasks", meta = (WorldContext = "WorldContextObject"))
    class UAzSpeechTaskBase* CreateSSMLToWavFileTask(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions SubscriptionOptions, const FAzSpeechSynthesisOptions SynthesisOptions, const FString& SynthesisSSML, const FString& FilePath, const FString& FileName);

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Tasks", meta = (WorldContext = "WorldContextObject"))
    class UAzSpeechTaskBase* CreateTextToAudioDataTask(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions SubscriptionOptions, const FAzSpeechSynthesisOptions SynthesisOptions, const FString& SynthesisText);

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Tasks", meta = (WorldContext = "WorldContextObject"))
    class UAzSpeechTaskBase* CreateTextToSoundWaveTask(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions SubscriptionOptions, const FAzSpeechSynthesisOptions SynthesisOptions, const FString& SynthesisText);

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Tasks", meta = (WorldContext = "WorldContextObject"))
    class UAzSpeechTaskBase* CreateTextToSpeechTask(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions SubscriptionOptions, const FAzSpeechSynthesisOptions SynthesisOptions, const FString& SynthesisText);

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Tasks", meta = (WorldContext = "WorldContextObject"))
    class UAzSpeechTaskBase* CreateTextToWavFileTask(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions SubscriptionOptions, const FAzSpeechSynthesisOptions SynthesisOptions, const FString& SynthesisText, const FString& FilePath, const FString& FileName);

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Tasks", meta = (WorldContext = "WorldContextObject"))
    class UAzSpeechTaskBase* CreateWavFileToTextTask(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions SubscriptionOptions, const FAzSpeechRecognitionOptions RecognitionOptions, const FString& FilePath, const FString& FileName, const FName PhraseListGroup = NAME_None);

private:
    void RegisterAzSpeechTask(class UAzSpeechTaskBase* const Task) const;
    void UnregisterAzSpeechTask(class UAzSpeechTaskBase* const Task) const;

    void ValidateRegisteredTasks() const;

    void DequeueExecutionQueue(const int64 QueueId) const;
    void OnQueueExecutionCompleted(const FAzSpeechTaskData Data, const int64 QueueId) const;

    void DequeueAudioExecutionQueue(const int64 QueueId) const;
    void OnQueueAudioExecutionCompleted(const FAzSpeechTaskData Data, const int64 QueueId) const;

public:
    UPROPERTY(BlueprintAssignable, Category = "AzSpeech | Management")
    FAzSpeechTaskRegistrationUpdate OnAzSpeechTaskRegistered;

    UPROPERTY(BlueprintAssignable, Category = "AzSpeech | Management")
    FAzSpeechTaskRegistrationUpdate OnAzSpeechTaskUnregistered;

    UPROPERTY(BlueprintAssignable, Category = "AzSpeech | Management")
    FAzSpeechTaskQueueExecutionProgress OnAzSpeechExecutionQueueProgressed;

private:
    mutable TArray<TWeakObjectPtr<class UAzSpeechTaskBase>> RegisteredTasks;

    // TMap doesnt support TQueue, so we use TArray instead
    typedef TPair<bool, TArray<TWeakObjectPtr<class UAzSpeechTaskBase>>> AzSpeechTaskQueueValue;
    mutable TMap<int64, AzSpeechTaskQueueValue> TaskQueueMap;

    // TMap doesnt support TQueue, so we use TArray instead
    mutable TMap<int64, TArray<TWeakObjectPtr<class UAzSpeechTaskBase>>> TaskAudioQueueMap;
};