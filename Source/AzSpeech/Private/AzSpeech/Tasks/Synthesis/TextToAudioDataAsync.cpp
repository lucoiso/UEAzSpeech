// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Synthesis/TextToAudioDataAsync.h"

#if WITH_EDITOR
#include <Editor.h>
#endif

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(TextToAudioDataAsync)
#endif

#if WITH_EDITOR
UTextToAudioDataAsync* UTextToAudioDataAsync::EditorTask(const FString& SynthesisText, const FString& Voice, const FString& Locale)
{
    UTextToAudioDataAsync* const NewAsyncTask = TextToAudioData_DefaultOptions(GEditor->GetEditorWorldContext().World(), SynthesisText, Voice, Locale);
    NewAsyncTask->bIsEditorTask = true;

    return NewAsyncTask;
}
#endif

UTextToAudioDataAsync* UTextToAudioDataAsync::TextToAudioData_DefaultOptions(UObject* const WorldContextObject, const FString& SynthesisText, const FString& Voice, const FString& Locale)
{
    return TextToAudioData_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechSynthesisOptions(*Locale, *Voice), SynthesisText);
}

UTextToAudioDataAsync* UTextToAudioDataAsync::TextToAudioData_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText)
{
    UTextToAudioDataAsync* const NewAsyncTask = NewObject<UTextToAudioDataAsync>();
    NewAsyncTask->WorldContextObject = WorldContextObject;
    NewAsyncTask->SynthesisText = SynthesisText;
    NewAsyncTask->SubscriptionOptions = SubscriptionOptions;
    NewAsyncTask->SynthesisOptions = SynthesisOptions;
    NewAsyncTask->bIsSSMLBased = false;
    NewAsyncTask->TaskName = *FString(__func__);

    NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

    return NewAsyncTask;
}

void UTextToAudioDataAsync::BroadcastFinalResult()
{
    FScopeLock Lock(&Mutex);

    if (!UAzSpeechTaskStatus::IsTaskActive(this))
    {
        return;
    }

    Super::BroadcastFinalResult();
    SynthesisCompleted.Broadcast(GetAudioData());

    SetReadyToDestroy();
}