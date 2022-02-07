#include "TextToVoiceAsync.h"

UTextToVoiceAsync* UTextToVoiceAsync::TextToVoiceAsync(const UObject* WorldContextObject, FString TextToConvert,
                                                       FString VoiceName, FAzureSpeechData Parameters)
{
	UTextToVoiceAsync* TextToVoiceAsync = NewObject<UTextToVoiceAsync>();
	TextToVoiceAsync->WorldContextObject = WorldContextObject;
	TextToVoiceAsync->TextToConvert = TextToConvert;
	TextToVoiceAsync->VoiceName = VoiceName;
	TextToVoiceAsync->Parameters = Parameters;

	return TextToVoiceAsync;
}

void UTextToVoiceAsync::Activate()
{
	AzureSpeech::AsyncTextToVoice(Parameters, TextToConvert, TaskCompleted, VoiceName);
}
