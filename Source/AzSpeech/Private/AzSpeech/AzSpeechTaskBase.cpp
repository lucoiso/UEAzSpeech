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
	FEditorDelegates::EndPIE.AddUObject(this, &UAzSpeechTaskBase::OnEndPIE);
#endif
}

void UAzSpeechTaskBase::StopAzSpeechTask()
{
#if WITH_EDITOR
	FEditorDelegates::EndPIE.RemoveAll(this);
#endif

	UE_LOG(LogAzSpeech, Display, TEXT("%s - Finishing AzSpeech Task"), *FString(__func__));
}

bool UAzSpeechTaskBase::StartAzureTaskWork_Internal()
{
	return AzSpeech::Internal::CheckAzSpeechSettings();
}

bool UAzSpeechTaskBase::CanBroadcast() const
{
	return IsValid(this);
}

#if WITH_EDITOR
void UAzSpeechTaskBase::OnEndPIE([[maybe_unused]] const bool bIsSimulating)
{
	StopAzSpeechTask();
}
#endif