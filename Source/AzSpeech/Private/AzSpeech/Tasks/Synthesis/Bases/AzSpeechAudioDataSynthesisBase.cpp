// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Synthesis/Bases/AzSpeechAudioDataSynthesisBase.h"
#include "AzSpeechInternalFuncs.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechAudioDataSynthesisBase)
#endif

namespace MicrosoftSpeech = Microsoft::CognitiveServices::Speech;

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

    AudioConfig = MicrosoftSpeech::Audio::AudioConfig::FromStreamOutput(MicrosoftSpeech::Audio::AudioOutputStream::CreatePullStream());
    StartSynthesisWork(AudioConfig);

    return true;
}