// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SSMLToWavFileAsync.h"

USSMLToWavFileAsync* USSMLToWavFileAsync::SSMLToWavFile(UObject* WorldContextObject, const FString& SynthesisSSML, const FString& FilePath, const FString& FileName)
{
	USSMLToWavFileAsync* const NewAsyncTask = NewObject<USSMLToWavFileAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SynthesisSSML;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->bIsSSMLBased = true;
	NewAsyncTask->TaskName = *FString(__func__);
	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}