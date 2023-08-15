// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeechPropertiesGetter.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechPropertiesGetter)
#endif

UAzSpeechPropertiesGetter::UAzSpeechPropertiesGetter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UAzSpeechPropertiesGetter::OnAvailableVoicesChanged(const TArray<FString>& Voices)
{
    OnAvailableVoicesUpdated.ExecuteIfBound(Voices);
    Destroy();
}

void UAzSpeechPropertiesGetter::SynthesisCompleted(const TArray<uint8>& AudioData)
{
    OnAudioDataGenerated.ExecuteIfBound(AudioData);
    Destroy();
}

void UAzSpeechPropertiesGetter::TaskFail()
{
    Destroy();
}

void UAzSpeechPropertiesGetter::Destroy()
{
    ClearFlags(RF_Standalone);

#if ENGINE_MAJOR_VERSION >= 5
    MarkAsGarbage();
#else
    MarkPendingKill();
#endif
}