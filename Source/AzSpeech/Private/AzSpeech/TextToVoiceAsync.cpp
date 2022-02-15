// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToVoiceAsync.h"
#include "AzSpeechWrapper.h"

UTextToVoiceAsync* UTextToVoiceAsync::TextToVoiceAsync(const UObject* WorldContextObject, const FString TextToConvert,
                                                       const FString VoiceName, const FAzSpeechData Parameters)
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
	FAzSpeechWrapper::Unreal_Cpp::AsyncTextToVoice(Parameters, TextToConvert, TaskCompleted, VoiceName);
}
