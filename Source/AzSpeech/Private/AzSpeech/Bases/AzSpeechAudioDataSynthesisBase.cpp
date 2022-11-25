// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Bases/AzSpeechAudioDataSynthesisBase.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"

bool UAzSpeechAudioDataSynthesisBase::StartAzureTaskWork()
{	
	if (!Super::StartAzureTaskWork())
	{
		return false;
	}
	
	if (AzSpeech::Internal::HasEmptyParam(SynthesisText))
	{
		return false;
	}
	
	FScopeLock Lock(&Mutex);

	const auto AudioConfig = Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromStreamOutput(Microsoft::CognitiveServices::Speech::Audio::AudioOutputStream::CreatePullStream());
	if (!InitializeSynthesizer(AudioConfig))
	{
		return false;
	}

	StartSynthesisWork();

	return true;
}

void UAzSpeechAudioDataSynthesisBase::OnSynthesisUpdate()
{	
	Super::OnSynthesisUpdate();

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(LastSynthesisResult->Reason))
	{
		FScopeLock Lock(&Mutex);
		OutputLastSynthesisResult(UAzSpeechHelper::IsAudioDataValid(GetLastSynthesizedAudioData()));
	}
}