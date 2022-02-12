// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/AzureSpeech

#include "TextToVoiceAsync.h"

UTextToVoiceAsync* UTextToVoiceAsync::TextToVoiceAsync(const UObject* WorldContextObject, const FString TextToConvert,
                                                       const FString VoiceName, const FAzureSpeechData Parameters)
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
