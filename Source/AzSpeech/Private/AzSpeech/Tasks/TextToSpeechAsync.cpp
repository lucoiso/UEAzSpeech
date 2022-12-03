// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/TextToSpeechAsync.h"

UTextToSpeechAsync* UTextToSpeechAsync::TextToSpeech(const UObject* WorldContextObject, const FString& SynthesisText, const FString& VoiceName, const FString& LanguageID)
{
	UTextToSpeechAsync* const NewAsyncTask = NewObject<UTextToSpeechAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SynthesisText ;
	NewAsyncTask->VoiceName = VoiceName;
	NewAsyncTask->LanguageID = LanguageID;
	NewAsyncTask->bIsSSMLBased = false;
	NewAsyncTask->TaskName = *FString(__func__);

	return NewAsyncTask;
}