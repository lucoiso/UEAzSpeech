// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "AzSpeech/Structures/AzSpeechSettingsOptions.h"
#include "LogAzSpeech.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_audio_config.h>
THIRD_PARTY_INCLUDES_END

#include "AzSpeechTaskBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAzSpeechTaskGenericDelegate);

/**
 *
 */
UCLASS(Abstract, NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UAzSpeechTaskBase : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

    friend class FAzSpeechRunnableBase;
    friend class UAzSpeechTaskStatus;
    friend class UAzSpeechEngineSubsystem;

public:
    virtual void Activate() override;

    UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (DisplayName = "Stop AzSpeech Task"))
    virtual void StopAzSpeechTask();

    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    const FName GetTaskName() const;

    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    const FAzSpeechSubscriptionOptions& GetSubscriptionOptions() const;

    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    void SetSubscriptionOptions(const FAzSpeechSubscriptionOptions& Options);

    virtual void SetReadyToDestroy() override;

protected:
    std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig> AudioConfig;

    TUniquePtr<class FAzSpeechRunnableBase> RunnableTask;
    FName TaskName = NAME_None;

    FAzSpeechSubscriptionOptions SubscriptionOptions;

    bool bIsSSMLBased = false;

    virtual bool StartAzureTaskWork();
    virtual void BroadcastFinalResult();

    mutable FCriticalSection Mutex;

#if WITH_EDITOR
    bool bIsEditorTask = false;
    bool bEndingPIE = false;

    virtual void PrePIEEnded(bool bIsSimulating);
#endif

    template <typename ReturnTy, typename ResultType>
    constexpr ReturnTy GetProperty(const ResultType& Result, const Microsoft::CognitiveServices::Speech::PropertyId ID)
    {
        const auto Property = Result->Properties.GetProperty(ID);
        if (Property.empty())
        {
            return ReturnTy();
        }

        if constexpr (std::is_same_v<ReturnTy, FString>)
        {
            return FString(UTF8_TO_TCHAR(Property.c_str()));
        }
        else if constexpr (std::is_same_v<ReturnTy, FName>)
        {
            return FName(UTF8_TO_TCHAR(Property.c_str()));
        }
        else if constexpr (std::is_same_v<ReturnTy, int32>)
        {
            return FCString::Atoi(*FString(UTF8_TO_TCHAR(Property.c_str())));
        }
        else if constexpr (std::is_same_v<ReturnTy, float>)
        {
            return FCString::Atof(*FString(UTF8_TO_TCHAR(Property.c_str())));
        }
        else if constexpr (std::is_same_v<ReturnTy, bool>)
        {
            return FCString::ToBool(*FString(UTF8_TO_TCHAR(Property.c_str())));
        }

        return ReturnTy();
    }

protected:

    typedef TDelegate<void(struct FAzSpeechTaskData)> FAzSpeechTaskGenericDelegate_Internal;

private:
    bool bIsTaskActive = false;
    bool bIsReadyToDestroy = false;

    FAzSpeechTaskGenericDelegate_Internal InternalOnTaskFinished;
};

UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UAzSpeechTaskStatus final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    static bool IsTaskActive(const UAzSpeechTaskBase* const Test);

    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    static bool IsTaskReadyToDestroy(const UAzSpeechTaskBase* const Test);

    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    static bool IsTaskStillValid(const UAzSpeechTaskBase* const Test);
};