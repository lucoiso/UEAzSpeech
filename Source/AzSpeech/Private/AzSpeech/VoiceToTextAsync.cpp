// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/VoiceToTextAsync.h"
#include "AzSpeechWrapper.h"

UVoiceToTextAsync* UVoiceToTextAsync::VoiceToTextAsync(const UObject* WorldContextObject,
                                                       const FAzSpeechData Parameters)
{
	UVoiceToTextAsync* VoiceToTextAsync = NewObject<UVoiceToTextAsync>();
	VoiceToTextAsync->WorldContextObject = WorldContextObject;
	VoiceToTextAsync->Parameters = Parameters;
	return VoiceToTextAsync;
}

void UVoiceToTextAsync::Activate()
{
	FAzSpeechWrapper::Unreal_Cpp::AsyncVoiceToText(Parameters, TaskCompleted);
}
