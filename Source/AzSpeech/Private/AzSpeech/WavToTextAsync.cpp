// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/WavToTextAsync.h"
#include "AzSpeechWrapper.h"

UWavToTextAsync* UWavToTextAsync::WavToTextAsync(const UObject* WorldContextObject, const FString FilePath, const FString FileName, FAzSpeechData Parameters)
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
	FAzSpeechWrapper::Unreal_Cpp::AsyncWavToText(FilePath, FileName, Parameters, TaskCompleted);
}