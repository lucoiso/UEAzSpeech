// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechAudioDataSynthesisBase.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechAudioDataSynthesisBase)
#endif

bool UAzSpeechAudioDataSynthesisBase::StartAzureTaskWork()
{	
	if (!Super::StartAzureTaskWork())
	{
		return false;
	}
	
	if (HasEmptyParameters(SynthesisText))
	{
		return false;
	}

	const auto AudioConfig = Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromStreamOutput(Microsoft::CognitiveServices::Speech::Audio::AudioOutputStream::CreatePullStream());
	StartSynthesisWork(AudioConfig);

	return true;
}