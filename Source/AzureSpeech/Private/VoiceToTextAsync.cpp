#include "VoiceToTextAsync.h"

UVoiceToTextAsync* UVoiceToTextAsync::VoiceToTextAsync(const UObject* WorldContextObject, FAzureSpeechData Parameters)
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
