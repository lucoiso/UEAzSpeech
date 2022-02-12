// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/AzureSpeech

#include "VoiceToTextAsync.h"

UVoiceToTextAsync* UVoiceToTextAsync::VoiceToTextAsync(const UObject* WorldContextObject,
                                                       const FAzureSpeechData Parameters)
{
	UVoiceToTextAsync* VoiceToTextAsync = NewObject<UVoiceToTextAsync>();
	VoiceToTextAsync->WorldContextObject = WorldContextObject;
	VoiceToTextAsync->Parameters = Parameters;
	return VoiceToTextAsync;
}

void UVoiceToTextAsync::Activate()
{
	AzureSpeech::AsyncVoiceToText(Parameters, TaskCompleted);
}
