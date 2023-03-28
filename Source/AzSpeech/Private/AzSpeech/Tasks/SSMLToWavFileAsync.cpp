// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/SSMLToWavFileAsync.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SSMLToWavFileAsync)
#endif

USSMLToWavFileAsync* USSMLToWavFileAsync::SSMLToWavFile_DefaultOptions(UObject* WorldContextObject, const FString& SynthesisSSML, const FString& FilePath, const FString& FileName)
{
	return SSMLToWavFile_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechSynthesisOptions(), SynthesisSSML, FilePath, FileName);
}

USSMLToWavFileAsync* USSMLToWavFileAsync::SSMLToWavFile_CustomOptions(UObject* WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML, const FString& FilePath, const FString& FileName)
{
	USSMLToWavFileAsync* const NewAsyncTask = NewObject<USSMLToWavFileAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SynthesisSSML;
	NewAsyncTask->SubscriptionOptions = SubscriptionOptions;
	NewAsyncTask->SynthesisOptions = SynthesisOptions;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->bIsSSMLBased = true;
	NewAsyncTask->TaskName = *FString(__func__);
	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}