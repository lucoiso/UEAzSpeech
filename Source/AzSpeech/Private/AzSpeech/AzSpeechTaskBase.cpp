// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechTaskBase.h"
#include "AzSpeechInternalFuncs.h"

void UAzSpeechTaskBase::Activate()
{
	Super::Activate();
	
	StartAzureTaskWork_Internal();
	
	FWorldDelegates::OnPostWorldCleanup.AddUObject(this, &UAzSpeechTaskBase::OnPostWorldCleanUp);
}

void UAzSpeechTaskBase::StopAzSpeechTask()
{
	FWorldDelegates::OnPostWorldCleanup.RemoveAll(this);

	bIsPendingDestruction = true;
	UE_LOG(LogAzSpeech, Display, TEXT("%s - Finishing AzSpeech Task"), *FString(__func__));
	
	if (CanDestroyTask())
	{
		SetReadyToDestroy();
	}
}

bool UAzSpeechTaskBase::StartAzureTaskWork_Internal()
{
	return AzSpeech::Internal::CheckAzSpeechSettings();
}

bool UAzSpeechTaskBase::CanBroadcast() const
{
	return IsValid(this) && !bIsPendingDestruction;
}

bool UAzSpeechTaskBase::CanDestroyTask() const
{
	return IsValid(this) && (!HasAnyFlags(RF_InternalPendingKill) || RegisteredWithGameInstance.IsValid());
}

void UAzSpeechTaskBase::OnPostWorldCleanUp(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s called."), *FString(__func__));
	
	StopAzSpeechTask();
}
