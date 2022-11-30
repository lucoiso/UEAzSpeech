// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/RecognitionMapCheckAsync.h"
#include "AzSpeech/AzSpeechRecognitionMap.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>

URecognitionMapCheckAsync* URecognitionMapCheckAsync::RecognitionMapCheckAsync(const UObject* WorldContextObject, const FString& InString, const FName GroupName, const bool bStopAtFirstTrigger)
{
	URecognitionMapCheckAsync* const NewAsyncTask = NewObject<URecognitionMapCheckAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->InputString = InString;
	NewAsyncTask->GroupName = GroupName;
	NewAsyncTask->bStopAtFirstTrigger = bStopAtFirstTrigger;
	NewAsyncTask->TaskName = *FString(__func__);

	return NewAsyncTask;
}

void URecognitionMapCheckAsync::Activate()
{
	Super::Activate();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		const int32 TaskResult = CheckRecognitionResult_Internal();
		AsyncTask(ENamedThreads::GameThread, [this, TaskResult] { BroadcastResult(TaskResult); });
	});
}

void URecognitionMapCheckAsync::BroadcastResult(const int32 Result)
{
	check(IsInGameThread());

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

const int32 URecognitionMapCheckAsync::CheckRecognitionResult_Internal()
{
	if (AzSpeech::Internal::HasEmptyParam(InputString, GroupName))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Invalid input string or group name"), *FString(__func__));
		return -1;
	}

	const FAzSpeechRecognitionMap InMap = AzSpeech::Internal::GetRecognitionMap(GroupName);
	FAzSpeechRecognitionData OutputResult(-1);
	uint32 MatchPoints = 0u;

	// Check if the string contains at least 1 requirement key
	bool bContainsRequirement = AzSpeech::Internal::HasEmptyParam(InMap.GlobalRequirementKeys);
	for (const FString& GlobalRequirementKey : InMap.GlobalRequirementKeys)
	{
		if (CheckStringContains_Internal("requirement", GlobalRequirementKey))
		{
			bContainsRequirement = true;
			break;
		}

		bContainsRequirement = false;
	}

	if (!bContainsRequirement)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Aborting check: String '%s' does not contains any requirement key from group %s"), *FString(__func__), *InputString, *GroupName.ToString());
		return -1;
	}

	// Check if the string contains a ignore key
	for (const FString& GlobalIgnoreKey : InMap.GlobalIgnoreKeys)
	{
		if (CheckStringContains_Internal("global ignore", GlobalIgnoreKey))
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
			bIgnore = CheckStringContains_Internal("ignore", Ignore);
		}

		if (bIgnore)
		{
			continue;
		}

		// Look for trigger keys in the input string and increment matching points if contains
		uint32 It_MatchPoints = 0u;
		for (const FString& Trigger : Iterator.TriggerKeys)
		{
			if (CheckStringContains_Internal("trigger", Trigger))
			{
				It_MatchPoints += Iterator.Weight;

				if (bStopAtFirstTrigger)
				{
					UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Returning first triggered key from group %s. Result: %d"), *FString(__func__), *GroupName.ToString(), OutputResult.Value);
					return Iterator.Value;
				}
			}
		}

		if (It_MatchPoints > MatchPoints)
		{
			MatchPoints = It_MatchPoints;
			OutputResult = Iterator;
		}
	}

	if (OutputResult.Value < 0 || MatchPoints == 0u)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Failed to find matching data in recognition map group %s"), *FString(__func__), *GroupName.ToString());
	}
	else
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Found matching data in recognition map group %s. Result: %d; Matching Points: %d"), *FString(__func__), *GroupName.ToString(), OutputResult.Value, MatchPoints);
	}

	return OutputResult.Value;
}

const bool URecognitionMapCheckAsync::CheckStringContains_Internal(const FString& KeyType, const FString& Key)
{
	if (AzSpeech::Internal::HasEmptyParam(Key))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Empty %s key in group %s"), *FString(__func__), *KeyType, *GroupName.ToString());
		return false;
	}

	const FString StringDelimiters = AzSpeech::Internal::GetStringDelimiters();

	const auto CheckDelimiter_Lambda = [this, &StringDelimiters, FuncName = __func__](const int32 Index)
	{
		if (InputString.IsValidIndex(Index))
		{
			const FString PreviousSubStr = InputString.Mid(Index, 1);
			const bool bResult = StringDelimiters.Contains(PreviousSubStr);

			UE_LOG(LogAzSpeech_Debugging, Display, TEXT("%s: Checking delimiter in string '%s' index %d. Result: %d"), *FString(FuncName), *InputString, Index, bResult);
			return bResult;
		}

		UE_LOG(LogAzSpeech_Debugging, Display, TEXT("%s: String '%s' does not contains index %d"), *FString(FuncName), *InputString, Index);
		return true;
	};

	const int32 Index = InputString.Find(Key, ESearchCase::IgnoreCase, ESearchDir::FromStart, -1);
	const bool bOutput = Index != INDEX_NONE && CheckDelimiter_Lambda(Index - 1) && CheckDelimiter_Lambda(Index + Key.Len());

	if (bOutput)
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: String '%s' contains the %s key '%s' from group %s"), *FString(__func__), *InputString, *KeyType, *Key, *GroupName.ToString());
	}
	else
	{
		UE_LOG(LogAzSpeech_Debugging, Error, TEXT("%s: String '%s' does not contains the %s key '%s' from group %s"), *FString(__func__), *InputString, *KeyType, *Key, *GroupName.ToString());
	}

	return bOutput;
}
