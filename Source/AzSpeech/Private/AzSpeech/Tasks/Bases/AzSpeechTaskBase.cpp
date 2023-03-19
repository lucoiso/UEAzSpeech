// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechTaskBase.h"
#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"
#include "AzSpeech/AzSpeechHelper.h"
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
	FScopeLock Lock(&Mutex);

	if (!UAzSpeechTaskStatus::IsTaskActive(this) || UAzSpeechTaskStatus::IsTaskReadyToDestroy(this))
	{
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Stopping task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
	bIsTaskActive = false;

	if (RunnableTask)
	{
		RunnableTask->StopAzSpeechRunnableTask();
	}
	
	SetReadyToDestroy();
}

const bool UAzSpeechTaskBase::IsUsingAutoLanguage() const
{
	return GetTaskOptions().LanguageID.ToString().Equals("Auto", ESearchCase::IgnoreCase);
}

const FName UAzSpeechTaskBase::GetTaskName() const
{
	return TaskName;
}

const FAzSpeechSettingsOptions UAzSpeechTaskBase::GetTaskOptions() const
{
	return TaskOptions;
}

void UAzSpeechTaskBase::SetReadyToDestroy()
{
	FScopeLock Lock(&Mutex);

	if (UAzSpeechTaskStatus::IsTaskReadyToDestroy(this))
	{
		return;
	}

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

	return UAzSpeechSettings::CheckAzSpeechSettings() && UAzSpeechTaskStatus::IsTaskStillValid(this);
}

void UAzSpeechTaskBase::BroadcastFinalResult()
{
	FScopeLock Lock(&Mutex);

	if (!bIsTaskActive)
	{
		return;
	}
	
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

FAzSpeechSettingsOptions UAzSpeechTaskBase::GetValidatedOptions(const FAzSpeechSettingsOptions& Options)
{
	FAzSpeechSettingsOptions Output = Options;
	Output.LanguageID = GetValidatedLanguageID(Options.LanguageID);
	Output.VoiceName = GetValidatedVoiceName(Options.VoiceName);

	return Output;
}

FName UAzSpeechTaskBase::GetValidatedLanguageID(const FName& Language)
{
	if (AzSpeech::Internal::HasEmptyParam(Language) || Language.ToString().Equals("Default", ESearchCase::IgnoreCase))
	{
		return UAzSpeechSettings::Get()->DefaultOptions.LanguageID;
	}

	return Language;
}

FName UAzSpeechTaskBase::GetValidatedVoiceName(const FName& Voice)
{
	if (AzSpeech::Internal::HasEmptyParam(Voice) || Voice.ToString().Equals("Default", ESearchCase::IgnoreCase))
	{
		return UAzSpeechSettings::Get()->DefaultOptions.VoiceName;
	}

	return Voice;
}

bool UAzSpeechTaskStatus::IsTaskActive(const UAzSpeechTaskBase* Test)
{
	return IsValid(Test) && Test->bIsTaskActive;
}

bool UAzSpeechTaskStatus::IsTaskReadyToDestroy(const UAzSpeechTaskBase* Test)
{
	return IsValid(Test) && Test->bIsReadyToDestroy;
}

bool UAzSpeechTaskStatus::IsTaskStillValid(const UAzSpeechTaskBase* Test)
{
	bool bOutput = IsValid(Test) && !IsTaskReadyToDestroy(Test);

#if WITH_EDITOR
	bOutput = bOutput && !Test->bEndingPIE;
#endif

	return bOutput;
}
