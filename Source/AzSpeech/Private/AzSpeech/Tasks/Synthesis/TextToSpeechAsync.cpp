// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Synthesis/TextToSpeechAsync.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(TextToSpeechAsync)
#endif

UTextToSpeechAsync* UTextToSpeechAsync::TextToSpeech_DefaultOptions(UObject* const WorldContextObject, const FString& SynthesisText, const FString& Voice, const FString& Locale)
{
    return TextToSpeech_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechSynthesisOptions(*Locale, *Voice), SynthesisText);
}

UTextToSpeechAsync* UTextToSpeechAsync::TextToSpeech_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText)
{
    UTextToSpeechAsync* const NewAsyncTask = NewObject<UTextToSpeechAsync>();
    NewAsyncTask->WorldContextObject = WorldContextObject;
    NewAsyncTask->SynthesisText = SynthesisText;
    NewAsyncTask->SubscriptionOptions = SubscriptionOptions;
    NewAsyncTask->SynthesisOptions = SynthesisOptions;
    NewAsyncTask->bIsSSMLBased = false;
    NewAsyncTask->TaskName = *FString(__func__);

    NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

    return NewAsyncTask;
}