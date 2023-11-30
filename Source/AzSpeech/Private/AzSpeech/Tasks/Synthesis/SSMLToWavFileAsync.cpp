// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Synthesis/SSMLToWavFileAsync.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SSMLToWavFileAsync)
#endif

USSMLToWavFileAsync* USSMLToWavFileAsync::SSMLToWavFile_DefaultOptions(UObject* const WorldContextObject, const FString& SynthesisSSML, const FString& FilePath, const FString& FileName)
{
    return SSMLToWavFile_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechSynthesisOptions(), SynthesisSSML, FilePath, FileName);
}

USSMLToWavFileAsync* USSMLToWavFileAsync::SSMLToWavFile_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML, const FString& FilePath, const FString& FileName)
{
    USSMLToWavFileAsync* const NewAsyncTask = NewObject<USSMLToWavFileAsync>();
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