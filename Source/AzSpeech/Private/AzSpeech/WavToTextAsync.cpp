// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/WavToTextAsync.h"
#include "AzSpeechWrapper.h"

UWavToTextAsync* UWavToTextAsync::WavToTextAsync(const UObject* WorldContextObject, FAzSpeechData Parameters, const FString FilePath, const FString FileName)
{
	UWavToTextAsync* WavToTextAsync = NewObject<UWavToTextAsync>();
	WavToTextAsync->WorldContextObject = WorldContextObject;
	WavToTextAsync->Parameters = Parameters;
	WavToTextAsync->FilePath = FilePath;
	WavToTextAsync->FileName = FileName;

	return WavToTextAsync;
}

void UWavToTextAsync::Activate()
{
	FAzSpeechWrapper::Unreal_Cpp::AsyncWavToText(Parameters, FilePath, FileName, TaskCompleted);
}