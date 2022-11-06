// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeechInternalFuncs.h"
#include "Async/Async.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

void UAzSpeechSynthesizerTaskBase::Activate()
{
	Super::Activate();

#if WITH_EDITOR
	FEditorDelegates::EndPIE.AddUObject(this, &UAzSpeechSynthesizerTaskBase::OnEndPIE);
#endif
}

void UAzSpeechSynthesizerTaskBase::StopAzSpeechTask()
{
#if WITH_EDITOR
	FEditorDelegates::EndPIE.RemoveAll(this);
#endif

	UE_LOG(LogAzSpeech, Display, TEXT("%s - Finishing AzSpeech Task"), *FString(__func__));

	if (!SynthesizerObject)
	{
		SetReadyToDestroy();
		return;
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&]
	{
		const TFuture<void> StopTaskWork = Async(EAsyncExecution::Thread, [=]() -> void
		{
			return SynthesizerObject->StopSpeakingAsync().get();
		});

		StopTaskWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout()));

		if (IsValid(this))
		{
			AsyncTask(ENamedThreads::GameThread, [this] {  SetReadyToDestroy(); });
		}
	});
}

const FAzSpeechVisemeData UAzSpeechSynthesizerTaskBase::GetLastVisemeData() const
{
	return LastVisemeData;
}

const bool UAzSpeechSynthesizerTaskBase::IsLastVisemeDataValid() const
{
	return LastVisemeData.IsValid();
}

bool UAzSpeechSynthesizerTaskBase::StartAzureTaskWork_Internal()
{
	return Super::StartAzureTaskWork_Internal();
}

void UAzSpeechSynthesizerTaskBase::CheckAndAddViseme()
{
	if (!SynthesizerObject)
	{
		return;
	}

	if (VisemeReceived.IsBound())
	{
		SynthesizerObject->VisemeReceived.Connect([this](const Microsoft::CognitiveServices::Speech::SpeechSynthesisVisemeEventArgs& VisemeArgs)
		{
			OnVisemeReceived(VisemeArgs);
		});
	}
}

void UAzSpeechSynthesizerTaskBase::OnVisemeReceived(const Microsoft::CognitiveServices::Speech::SpeechSynthesisVisemeEventArgs& VisemeEventArgs)
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s - Viseme Id: %s"), *FString(__func__), *FString::FromInt(VisemeEventArgs.VisemeId));
	UE_LOG(LogAzSpeech, Display, TEXT("%s - Viseme Audio Offset: %s"), *FString(__func__), *FString::FromInt(VisemeEventArgs.AudioOffset));

	const FString VisemeAnimation_UEStr = UTF8_TO_TCHAR(VisemeEventArgs.Animation.c_str());
	UE_LOG(LogAzSpeech, Display, TEXT("%s - Viseme Animation: %s"), *FString(__func__), *VisemeAnimation_UEStr);
	
	LastVisemeData = FAzSpeechVisemeData(VisemeEventArgs.VisemeId, VisemeEventArgs.AudioOffset, VisemeAnimation_UEStr);
	VisemeReceived.Broadcast(LastVisemeData);
}

#if WITH_EDITOR
void UAzSpeechSynthesizerTaskBase::OnEndPIE([[maybe_unused]] const bool bIsSimulating)
{
	StopAzSpeechTask();
}
#endif