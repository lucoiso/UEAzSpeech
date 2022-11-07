// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeechInternalFuncs.h"
#include "Async/Async.h"

void UAzSpeechSynthesizerTaskBase::Activate()
{
	Super::Activate();
}

void UAzSpeechSynthesizerTaskBase::StopAzSpeechTask()
{	
	Super::StopAzSpeechTask();

	if (!SynthesizerObject)
	{
		SetReadyToDestroy();
		return;
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&]
	{
		const TFuture<void> StopTaskWork = Async(EAsyncExecution::Thread, [=]() -> void
		{
			VisemeReceived.RemoveAll(this);
	
			return SynthesizerObject->StopSpeakingAsync().get();
		});

		StopTaskWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout()));

		if (IsValid(this))
		{
			AsyncTask(ENamedThreads::GameThread, [this] { SetReadyToDestroy(); });
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
		
	const int64 AudioOffsetMs = VisemeEventArgs.AudioOffset / 10000;
	UE_LOG(LogAzSpeech, Display, TEXT("%s - Viseme Audio Offset: %sms"), *FString(__func__), *FString::FromInt(AudioOffsetMs));

	const FString VisemeAnimation_UEStr = UTF8_TO_TCHAR(VisemeEventArgs.Animation.c_str());
	UE_LOG(LogAzSpeech, Display, TEXT("%s - Viseme Animation: %s"), *FString(__func__), *VisemeAnimation_UEStr);
	
	LastVisemeData = FAzSpeechVisemeData(VisemeEventArgs.VisemeId, AudioOffsetMs, VisemeAnimation_UEStr);
	AsyncTask(ENamedThreads::GameThread, [=]() { if (CanBroadcast()) { VisemeReceived.Broadcast(LastVisemeData); } });	
}