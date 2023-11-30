// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Synthesis/SSMLToAudioDataAsync.h"

#if WITH_EDITOR
#include <Editor.h>
#endif

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SSMLToAudioDataAsync)
#endif

#if WITH_EDITOR
USSMLToAudioDataAsync* USSMLToAudioDataAsync::EditorTask(const FString& SynthesisSSML)
{
    USSMLToAudioDataAsync* const NewAsyncTask = SSMLToAudioData_DefaultOptions(GEditor->GetEditorWorldContext().World(), SynthesisSSML);
    NewAsyncTask->bIsEditorTask = true;

    return NewAsyncTask;
}
#endif

USSMLToAudioDataAsync* USSMLToAudioDataAsync::SSMLToAudioData_DefaultOptions(UObject* const WorldContextObject, const FString& SynthesisSSML)
{
    return SSMLToAudioData_CustomOptions(WorldContextObject, FAzSpeechSubscriptionOptions(), FAzSpeechSynthesisOptions(), SynthesisSSML);
}

USSMLToAudioDataAsync* USSMLToAudioDataAsync::SSMLToAudioData_CustomOptions(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML)
{
    USSMLToAudioDataAsync* const NewAsyncTask = NewObject<USSMLToAudioDataAsync>();
    NewAsyncTask->WorldContextObject = WorldContextObject;
    NewAsyncTask->SubscriptionOptions = SubscriptionOptions;
    NewAsyncTask->SynthesisOptions = SynthesisOptions;
    NewAsyncTask->SynthesisText = SynthesisSSML;
    NewAsyncTask->bIsSSMLBased = true;
    NewAsyncTask->TaskName = *FString(__func__);

    NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

    return NewAsyncTask;
}

void USSMLToAudioDataAsync::BroadcastFinalResult()
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