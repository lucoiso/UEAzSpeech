// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechTaskBase.h"
#include "AzSpeechInternalFuncs.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

void UAzSpeechTaskBase::Activate()
{
	Super::Activate();
	
	StartAzureTaskWork_Internal();

#if WITH_EDITOR
	FEditorDelegates::PrePIEEnded.AddUObject(this, &UAzSpeechTaskBase::PrePIEEnded);
#endif
}

void UAzSpeechTaskBase::StopAzSpeechTask()
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s: Finishing AzSpeech Task"), *FString(__func__));
	
	if (UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		SetReadyToDestroy();
		ClearBindings();
	}
}

const bool UAzSpeechTaskBase::IsTaskStillValid(const UAzSpeechTaskBase* Test)
{
	return IsValid(Test) && !Test->bIsReadyToDestroy;
}

bool UAzSpeechTaskBase::StartAzureTaskWork_Internal()
{
	return AzSpeech::Internal::CheckAzSpeechSettings();
}

void UAzSpeechTaskBase::SetReadyToDestroy()
{
	bIsReadyToDestroy = true;
	Super::SetReadyToDestroy();
}

#if WITH_EDITOR
void UAzSpeechTaskBase::PrePIEEnded(bool bIsSimulating)
{
	FEditorDelegates::PrePIEEnded.RemoveAll(this);

	UE_LOG(LogAzSpeech, Display, TEXT("%s called."), *FString(__func__));
	
	if (UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		StopAzSpeechTask();
	}
}
#endif