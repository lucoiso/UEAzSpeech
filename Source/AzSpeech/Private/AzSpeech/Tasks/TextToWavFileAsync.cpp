// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/TextToWavFileAsync.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(TextToWavFileAsync)
#endif

UTextToWavFileAsync* UTextToWavFileAsync::TextToWavFile_DefaultOptions(UObject* WorldContextObject, const FString& SynthesisText, const FString& FilePath, const FString& FileName, const FString& VoiceName, const FString& LanguageID)
{
	return TextToWavFile_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechSynthesisOptions(*LanguageID, *VoiceName), SynthesisText, FilePath, FileName);
}

UTextToWavFileAsync* UTextToWavFileAsync::TextToWavFile_CustomOptions(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText, const FString& FilePath, const FString& FileName)
{
	UTextToWavFileAsync* const NewAsyncTask = NewObject<UTextToWavFileAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SynthesisText;
	NewAsyncTask->SubscriptionOptions = SubscriptionOptions;
	NewAsyncTask->SynthesisOptions = SynthesisOptions;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->bIsSSMLBased = false;
	NewAsyncTask->TaskName = *FString(__func__);
	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}