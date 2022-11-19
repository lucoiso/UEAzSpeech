// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/TextToWavFileAsync.h"

UTextToWavFileAsync* UTextToWavFileAsync::TextToWavFile(const UObject* WorldContextObject, const FString& TextToConvert, const FString& FilePath, const FString& FileName, const FString& VoiceName, const FString& LanguageId)
{
	UTextToWavFileAsync* const NewAsyncTask = NewObject<UTextToWavFileAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = TextToConvert;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->VoiceName = VoiceName;
	NewAsyncTask->LanguageId = LanguageId;
	NewAsyncTask->bIsSSMLBased = false;

	return NewAsyncTask;
}