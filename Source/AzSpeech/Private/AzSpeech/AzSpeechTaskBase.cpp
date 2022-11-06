// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechTaskBase.h"
#include "AzSpeechInternalFuncs.h"

void UAzSpeechTaskBase::Activate()
{
	Super::Activate();
	
	StartAzureTaskWork_Internal();
}

bool UAzSpeechTaskBase::StartAzureTaskWork_Internal()
{
	return AzSpeech::Internal::CheckAzSpeechSettings();
}

bool UAzSpeechTaskBase::CanBroadcast() const
{
	return IsValid(this) && IsValid(GetWorld());
}
