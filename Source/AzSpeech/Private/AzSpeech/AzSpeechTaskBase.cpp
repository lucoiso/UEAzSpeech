// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechTaskBase.h"

void UAzSpeechTaskBase::Activate()
{
	Super::Activate();
	
	StartAzureTaskWork_Internal();
}

void UAzSpeechTaskBase::StartAzureTaskWork_Internal()
{
}
