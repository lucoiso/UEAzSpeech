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
	
	if (VisemeReceived.IsBound())
	{
		VisemeReceived.RemoveAll(this);
	}

	if (SynthesizerObject->VisemeReceived.IsConnected())
	{
		SynthesizerObject->VisemeReceived.DisconnectAll();
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, FuncName = __func__]
	{
		if (SynthesizerObject)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("%s - Trying to stop current synthesizer task..."), *FString(FuncName));

			switch (SynthesizerObject->StopSpeakingAsync().wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout())))
			{
				case std::future_status::ready:
					UE_LOG(LogAzSpeech, Display, TEXT("%s - Stop finished with status: Ready"), *FString(FuncName));
					break;

				case std::future_status::deferred:
					UE_LOG(LogAzSpeech, Display, TEXT("%s - Stop finished with status: Deferred"), *FString(FuncName));
					break;

				case std::future_status::timeout:
					UE_LOG(LogAzSpeech, Display, TEXT("%s - Stop finished with status: Time Out"), *FString(FuncName));
					break;

				default: break;
			}

			SynthesizerObject.reset();
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

void UAzSpeechSynthesizerTaskBase::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
}

void UAzSpeechSynthesizerTaskBase::EnableVisemeOutput()
{
	if (!AzSpeech::Internal::GetPluginSettings()->bEnableViseme)
	{
		return;
	}

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

	if (CanBroadcast())
	{
		AsyncTask(ENamedThreads::GameThread, [=]() { VisemeReceived.Broadcast(LastVisemeData); });
	}
}