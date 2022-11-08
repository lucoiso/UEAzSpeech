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
		return;
	}
	
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]
	{
		if (!Mutex.TryLock())
		{
			return;
		}

		SynthesizerObject->StopSpeakingAsync().wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
		SynthesizerObject.reset();
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

void UAzSpeechSynthesizerTaskBase::ClearBindings()
{
	if (VisemeReceived.IsBound())
	{
		VisemeReceived.RemoveAll(this);
	}

	if (!SynthesizerObject)
	{
		return;
	}

	Disconecter_T(SynthesizerObject->VisemeReceived);
	Disconecter_T(SynthesizerObject->Synthesizing);
	Disconecter_T(SynthesizerObject->SynthesisStarted);
	Disconecter_T(SynthesizerObject->SynthesisCompleted);
	Disconecter_T(SynthesizerObject->SynthesisCanceled);
}

void UAzSpeechSynthesizerTaskBase::EnableVisemeOutput()
{
	if (VisemeReceived.IsBound())
	{
		SynthesizerObject->VisemeReceived.Connect([this](const Microsoft::CognitiveServices::Speech::SpeechSynthesisVisemeEventArgs& VisemeArgs)
		{
			OnVisemeReceived(VisemeArgs);
		});
	}
}

void UAzSpeechSynthesizerTaskBase::ApplyExtraSettings()
{
	Super::ApplyExtraSettings();

	if (!SynthesizerObject)
	{
		return;
	}

	if (AzSpeech::Internal::GetPluginSettings()->bEnableViseme)
	{
		EnableVisemeOutput();
	}

	const auto SynthesisUpdate_Lambda = [this](const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
	{
		OnSynthesisUpdate(SynthesisEventArgs);
	};

	SynthesizerObject->Synthesizing.Connect(SynthesisUpdate_Lambda);
	SynthesizerObject->SynthesisStarted.Connect(SynthesisUpdate_Lambda);
	SynthesizerObject->SynthesisCompleted.Connect(SynthesisUpdate_Lambda);
	SynthesizerObject->SynthesisCanceled.Connect(SynthesisUpdate_Lambda);
}

void UAzSpeechSynthesizerTaskBase::OnVisemeReceived(const Microsoft::CognitiveServices::Speech::SpeechSynthesisVisemeEventArgs& VisemeEventArgs)
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s: Viseme Id: %s"), *FString(__func__), *FString::FromInt(VisemeEventArgs.VisemeId));

	const int64 AudioOffsetMs = VisemeEventArgs.AudioOffset / 10000;
	UE_LOG(LogAzSpeech, Display, TEXT("%s: Viseme Audio Offset: %sms"), *FString(__func__), *FString::FromInt(AudioOffsetMs));

	const FString VisemeAnimation_UEStr = UTF8_TO_TCHAR(VisemeEventArgs.Animation.c_str());
	UE_LOG(LogAzSpeech, Display, TEXT("%s: Viseme Animation: %s"), *FString(__func__), *VisemeAnimation_UEStr);

	LastVisemeData = FAzSpeechVisemeData(VisemeEventArgs.VisemeId, AudioOffsetMs, VisemeAnimation_UEStr);

	if (UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		AsyncTask(ENamedThreads::GameThread, [=]() { VisemeReceived.Broadcast(LastVisemeData); });
	}
}

void UAzSpeechSynthesizerTaskBase::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	if (SynthesisEventArgs.Result->Reason != ResultReason::SynthesizingAudio)
	{
		AzSpeech::Internal::ProcessSynthesisResult(SynthesisEventArgs.Result);
	}
}

void UAzSpeechSynthesizerTaskBase::OutputSynthesisResult(const bool bSuccess) const
{
	if (bSuccess)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Result: Success"), *FString(__func__));
	}
	else if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Result: Canceled"), *FString(__func__));
	}
	else
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Result: Failed"), *FString(__func__));
	}
}

const TArray<uint8> UAzSpeechSynthesizerTaskBase::GetUnrealStreamResult(const std::vector<uint8_t>& InBuffer)
{
	TArray<uint8> OutputArr;
	for (const uint8_t& i : InBuffer)
	{
		OutputArr.Add(static_cast<uint8>(i));
	}

	return OutputArr;
}
