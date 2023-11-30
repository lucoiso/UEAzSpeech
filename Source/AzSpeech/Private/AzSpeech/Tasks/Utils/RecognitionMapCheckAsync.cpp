// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Utils/RecognitionMapCheckAsync.h"
#include "AzSpeech/Structures/AzSpeechRecognitionMap.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(RecognitionMapCheckAsync)
#endif

URecognitionMapCheckAsync* URecognitionMapCheckAsync::RecognitionMapCheckAsync(UObject* const WorldContextObject, const FString& InString, const FName& GroupName, const bool bStopAtFirstTrigger)
{
    URecognitionMapCheckAsync* const NewAsyncTask = NewObject<URecognitionMapCheckAsync>();
    NewAsyncTask->InputString = InString;
    NewAsyncTask->GroupName = GroupName;
    NewAsyncTask->bStopAtFirstTrigger = bStopAtFirstTrigger;
    NewAsyncTask->TaskName = *FString(__func__);

    NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

    return NewAsyncTask;
}

void URecognitionMapCheckAsync::Activate()
{
    Super::Activate();

    UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Activating task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
        [this]
        {
            const int32 TaskResult = CheckRecognitionResult();
            AsyncTask(ENamedThreads::GameThread,
                [this, TaskResult]
                {
                    BroadcastResult(TaskResult);
                }
            );
        }
    );
}

void URecognitionMapCheckAsync::SetReadyToDestroy()
{
    UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Setting task as Ready to Destroy"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

    Super::SetReadyToDestroy();
}

void URecognitionMapCheckAsync::BroadcastResult(const int32 Result)
{
    check(IsInGameThread());

    UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Task completed. Broadcasting result: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), Result);

    if (Result < 0)
    {
        NotFound.Broadcast();
    }
    else
    {
        Found.Broadcast(Result);
    }

    if (NotFound.IsBound())
    {
        NotFound.Clear();
    }

    if (Found.IsBound())
    {
        Found.Clear();
    }

    SetReadyToDestroy();
}

const int32 URecognitionMapCheckAsync::CheckRecognitionResult() const
{
    if (AzSpeech::Internal::HasEmptyParam(InputString, GroupName))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Invalid input string or group name"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
        return -1;
    }

    const FAzSpeechRecognitionMap InMap = GetRecognitionMap(GroupName);
    FAzSpeechRecognitionData OutputResult(-1);
    uint32 MatchPoints = 0u;

    // Check if the string contains at least 1 requirement key
    bool bContainsRequirement = AzSpeech::Internal::HasEmptyParam(InMap.GlobalRequirementKeys);
    for (const FString& GlobalRequirementKey : InMap.GlobalRequirementKeys)
    {
        if (CheckStringContains("requirement", GlobalRequirementKey))
        {
            bContainsRequirement = true;
            break;
        }

        bContainsRequirement = false;
    }

    if (!bContainsRequirement)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Aborting check: String '%s' does not contains any requirement key from group %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *InputString, *GroupName.ToString());
        return -1;
    }

    // Check if the string contains a ignore key
    for (const FString& GlobalIgnoreKey : InMap.GlobalIgnoreKeys)
    {
        if (CheckStringContains("global ignore", GlobalIgnoreKey))
        {
            return -1;
        }
    }

    // Look for trigger keys in the string
    for (const FAzSpeechRecognitionData& Iterator : InMap.Data)
    {
        // Check for local ignore keys before trying to compare the trigger
        bool bIgnore = false;
        for (const FString& Ignore : Iterator.IgnoreKeys)
        {
            bIgnore = CheckStringContains("ignore", Ignore);
        }

        if (bIgnore)
        {
            continue;
        }

        // Look for trigger keys in the input string and increment matching points if contains
        uint32 It_MatchPoints = 0u;
        for (const FString& Trigger : Iterator.TriggerKeys)
        {
            if (CheckStringContains("trigger", Trigger))
            {
                It_MatchPoints += Iterator.Weight;

                if (bStopAtFirstTrigger)
                {
                    UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Returning first triggered key from group %s. Result: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *GroupName.ToString(), OutputResult.Value);
                    return Iterator.Value;
                }
            }
        }

        // If the current iterated object has more point that the cached one, replace the output object
        if (It_MatchPoints > MatchPoints)
        {
            MatchPoints = It_MatchPoints;
            OutputResult = Iterator;
        }
    }

    if (OutputResult.Value < 0 || MatchPoints == 0u)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Failed to find matching data in recognition map group %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *GroupName.ToString());
    }
    else
    {
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Found matching data in recognition map group %s. Result: %d; Matching Points: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *GroupName.ToString(), OutputResult.Value, MatchPoints);
    }

    return OutputResult.Value;
}

const bool URecognitionMapCheckAsync::CheckStringContains(const FString& KeyType, const FString& Key) const
{
    if (AzSpeech::Internal::HasEmptyParam(Key))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Empty %s key in group %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *KeyType, *GroupName.ToString());
        return false;
    }

    const int32 Index = InputString.Find(Key, ESearchCase::IgnoreCase, ESearchDir::FromStart, -1);
    const bool bOutput = Index != INDEX_NONE && CheckStringDelimiters(Index - 1) && CheckStringDelimiters(Index + Key.Len());

    if (bOutput)
    {
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: String '%s' contains the %s key '%s' from group %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *InputString, *KeyType, *Key, *GroupName.ToString());
    }
    else
    {
        UE_LOG(LogAzSpeech_Debugging, Error, TEXT("Task: %s (%d); Function: %s; Message: String '%s' does not contains the %s key '%s' from group %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *InputString, *KeyType, *Key, *GroupName.ToString());
    }

    return bOutput;
}

const FName URecognitionMapCheckAsync::GetStringDelimiters() const
{
    if (const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get())
    {
        return Settings->StringDelimiters;
    }

    return FName(" ,.;:[]{}!'\"?");
}

const bool URecognitionMapCheckAsync::CheckStringDelimiters(const int32 Index) const
{
    const FString StringDelimiters = GetStringDelimiters().ToString();

    if (InputString.IsValidIndex(Index))
    {
        const FString PreviousSubStr = InputString.Mid(Index, 1);
        const bool bResult = StringDelimiters.Contains(PreviousSubStr);

        UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Checking delimiter in string '%s' index %d. Result: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *InputString, Index, bResult);
        return bResult;
    }

    UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: String '%s' does not contains index %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *InputString, Index);
    return true;
}

const FAzSpeechRecognitionMap URecognitionMapCheckAsync::GetRecognitionMap(const FName& InGroup) const
{
    if (const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get())
    {
        return AzSpeech::Internal::GetDataFromMapGroup<FAzSpeechRecognitionMap, FAzSpeechRecognitionMap>(InGroup, Settings->RecognitionMap);
    }

    return FAzSpeechRecognitionMap();
}