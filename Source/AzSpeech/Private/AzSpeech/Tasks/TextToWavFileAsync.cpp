// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/TextToWavFileAsync.h"

UTextToWavFileAsync* UTextToWavFileAsync::TextToWavFile(UObject* WorldContextObject, const FString& SynthesisText, const FString& FilePath, const FString& FileName, const FString& VoiceName, const FString& LanguageID)
{
	UTextToWavFileAsync* const NewAsyncTask = NewObject<UTextToWavFileAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SynthesisText;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->VoiceName = VoiceName;
	NewAsyncTask->LanguageID = LanguageID;
	NewAsyncTask->bIsSSMLBased = false;
	NewAsyncTask->TaskName = *FString(__func__);
	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}