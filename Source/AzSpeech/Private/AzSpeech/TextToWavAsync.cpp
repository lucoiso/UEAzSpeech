// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToWavAsync.h"
#include "AzSpeechWrapper.h"

UTextToWavAsync* UTextToWavAsync::TextToWavAsync(const UObject* WorldContextObject, const FString TextToConvert, 
												const FString FilePath, const FString FileName, 
												const FString VoiceName, const FAzSpeechData Parameters)
{
	UTextToWavAsync* TextToWavAsync = NewObject<UTextToWavAsync>();
	TextToWavAsync->WorldContextObject = WorldContextObject;
	TextToWavAsync->TextToConvert = TextToConvert;
	TextToWavAsync->VoiceName = VoiceName;
	TextToWavAsync->FilePath = FilePath;
	TextToWavAsync->FileName = FileName;
	TextToWavAsync->Parameters = Parameters;

	return TextToWavAsync;
}

void UTextToWavAsync::Activate()
{
	FAzSpeechWrapper::Unreal_Cpp::AsyncTextToWav(Parameters, TextToConvert, TaskCompleted, VoiceName, FilePath, FileName);
}
