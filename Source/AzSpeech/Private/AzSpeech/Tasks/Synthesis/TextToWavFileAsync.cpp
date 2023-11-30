// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Synthesis/TextToWavFileAsync.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(TextToWavFileAsync)
#endif

UTextToWavFileAsync* UTextToWavFileAsync::TextToWavFile_DefaultOptions(UObject* const WorldContextObject, const FString& SynthesisText, const FString& FilePath, const FString& FileName, const FString& Voice, const FString& Locale)
{
    return TextToWavFile_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechSynthesisOptions(*Locale, *Voice), SynthesisText, FilePath, FileName);
}

UTextToWavFileAsync* UTextToWavFileAsync::TextToWavFile_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText, const FString& FilePath, const FString& FileName)
{
    UTextToWavFileAsync* const NewAsyncTask = NewObject<UTextToWavFileAsync>();
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