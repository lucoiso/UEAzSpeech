// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechTaskBase.h"
#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "LogAzSpeech.h"

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

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Activating task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

	ValidateLanguageID();

	bIsTaskActive = true;
	
	Super::Activate();
	
	if (!StartAzureTaskWork())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%d); Function: %s; Message: Failed to activate task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
		SetReadyToDestroy();
		
		return;
	}

#if WITH_EDITOR
	FEditorDelegates::PrePIEEnded.AddUObject(this, &UAzSpeechTaskBase::PrePIEEnded);
#endif
}

void UAzSpeechTaskBase::StopAzSpeechTask()
{
	if (!IsTaskActive(this) || IsTaskReadyToDestroy(this))
	{
		return;
	}

	FScopeLock Lock(&Mutex);

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Stopping task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
	bIsTaskActive = false;

	if (RunnableTask)
	{
		RunnableTask->StopAzSpeechRunnableTask();
	}

	BroadcastFinalResult();
	
	SetReadyToDestroy();
}

const bool UAzSpeechTaskBase::IsTaskActive(const UAzSpeechTaskBase* Test)
{
	return IsValid(Test) && Test->bIsTaskActive;
}

const bool UAzSpeechTaskBase::IsTaskReadyToDestroy(const UAzSpeechTaskBase* Test)
{
	return IsValid(Test) && Test->bIsReadyToDestroy;
}

const bool UAzSpeechTaskBase::IsTaskStillValid(const UAzSpeechTaskBase* Test)
{
	bool bOutput = IsValid(Test) && !IsTaskReadyToDestroy(Test);

#if WITH_EDITOR
	bOutput = bOutput && !Test->bEndingPIE;
#endif

	return bOutput;
}

const bool UAzSpeechTaskBase::IsUsingAutoLanguage() const
{
	return LanguageID.Equals("Auto", ESearchCase::IgnoreCase);
}

const FString UAzSpeechTaskBase::GetTaskName() const
{
	return TaskName.ToString();
}

const FString UAzSpeechTaskBase::GetLanguageID() const
{
	return LanguageID;
}

void UAzSpeechTaskBase::SetReadyToDestroy()
{
	if (IsTaskReadyToDestroy(this))
	{
		return;
	}

	FScopeLock Lock(&Mutex);

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Setting task as Ready to Destroy"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
	bIsReadyToDestroy = true;

#if WITH_EDITOR
	if (FEditorDelegates::PrePIEEnded.IsBoundToObject(this))
	{
		FEditorDelegates::PrePIEEnded.RemoveAll(this);
	}
#endif
	
	if (RunnableTask)
	{
		RunnableTask->StopAzSpeechRunnableTask();
	}

	Super::SetReadyToDestroy();
}

bool UAzSpeechTaskBase::StartAzureTaskWork()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Starting Azure SDK task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

	return UAzSpeechSettings::CheckAzSpeechSettings() && IsTaskStillValid(this);
}

void UAzSpeechTaskBase::BroadcastFinalResult()
{
	check(IsInGameThread());
	
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Task completed, broadcasting final result"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

	bIsTaskActive = false;
}

#if WITH_EDITOR
void UAzSpeechTaskBase::PrePIEEnded(bool bIsSimulating)
{
	if (!IsTaskStillValid(this))
	{
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Trying to finish task due to PIE end"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
	
	bEndingPIE = true;
	StopAzSpeechTask();
}
#endif

void UAzSpeechTaskBase::ValidateLanguageID()
{
	if (bIsSSMLBased)
	{
		return;
	}

	const auto Settings = UAzSpeechSettings::GetAzSpeechKeys();
	if (HasEmptyParameters(LanguageID) || LanguageID.Equals("Default", ESearchCase::IgnoreCase))
	{
		LanguageID = UTF8_TO_TCHAR(Settings.at(2).c_str());
	}
}