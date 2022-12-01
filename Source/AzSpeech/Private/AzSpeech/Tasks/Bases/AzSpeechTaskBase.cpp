// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechTaskBase.h"
#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"

#if WITH_EDITOR
#include <Editor.h>
#endif

void UAzSpeechTaskBase::Activate()
{
	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Activating task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

	AzSpeech::Internal::GetLanguageID(LanguageId);

	bIsTaskActive = true;

	Super::Activate();
	StartAzureTaskWork();

#if WITH_EDITOR
	FEditorDelegates::PrePIEEnded.AddUObject(this, &UAzSpeechTaskBase::PrePIEEnded);
#endif
}

void UAzSpeechTaskBase::StopAzSpeechTask()
{
	if (!IsTaskActive() || IsTaskReadyToDestroy())
	{
		return;
	}

	FScopeLock Lock(&Mutex);

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Finishing task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
	bIsTaskActive = false;

	if (RunnableTask)
	{
		RunnableTask->StopAzSpeechRunnableTask();
	}

	BroadcastFinalResult();
	
	SetReadyToDestroy();
}

bool UAzSpeechTaskBase::IsTaskActive() const
{
	return bIsTaskActive;
}

bool UAzSpeechTaskBase::IsTaskReadyToDestroy() const
{
	return bIsReadyToDestroy;
}

const bool UAzSpeechTaskBase::IsTaskStillValid(const UAzSpeechTaskBase* Test)
{
	bool bOutput = IsValid(Test) && !Test->IsTaskReadyToDestroy();

#if WITH_EDITOR
	bOutput = bOutput && !Test->bEndingPIE;
#endif

	return bOutput;
}

const bool UAzSpeechTaskBase::IsUsingAutoLanguage() const
{
	return LanguageId.Equals("Auto", ESearchCase::IgnoreCase);
}

const FString UAzSpeechTaskBase::GetTaskName() const
{
	return TaskName.ToString();
}

const FString UAzSpeechTaskBase::GetLanguageID() const
{
	return LanguageId;
}

void UAzSpeechTaskBase::SetReadyToDestroy()
{
	if (IsTaskReadyToDestroy())
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

	return AzSpeech::Internal::CheckAzSpeechSettings() && IsTaskStillValid(this);
}

void UAzSpeechTaskBase::BroadcastFinalResult()
{
	check(IsInGameThread());
	
	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Task completed, broadcasting final result"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

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