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

const TArray<uint8> UAzSpeechSynthesizerTaskBase::GetLastSynthesizedStream() const
{
	TArray<uint8> OutputArr;
	for (const uint8_t& i : LastSynthesizedBuffer)
	{
		OutputArr.Add(static_cast<uint8>(i));
	}

	return OutputArr;
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

	SignalDisconecter_T(SynthesizerObject->VisemeReceived);
	SignalDisconecter_T(SynthesizerObject->Synthesizing);
	SignalDisconecter_T(SynthesizerObject->SynthesisStarted);
	SignalDisconecter_T(SynthesizerObject->SynthesisCompleted);
	SignalDisconecter_T(SynthesizerObject->SynthesisCanceled);
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
	const int64 AudioOffsetMs = VisemeEventArgs.AudioOffset / 10000;
	const FString VisemeAnimation_UEStr = UTF8_TO_TCHAR(VisemeEventArgs.Animation.c_str());

	if (AzSpeech::Internal::GetPluginSettings()->bEnableRuntimeDebug)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Viseme Id: %s"), *FString(__func__), *FString::FromInt(VisemeEventArgs.VisemeId));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Viseme Audio Offset: %sms"), *FString(__func__), *FString::FromInt(AudioOffsetMs));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Viseme Animation: %s"), *FString(__func__), *VisemeAnimation_UEStr);
	}

	LastVisemeData = FAzSpeechVisemeData(VisemeEventArgs.VisemeId, AudioOffsetMs, VisemeAnimation_UEStr);
	VisemeReceived.Broadcast(LastVisemeData);
}

void UAzSpeechSynthesizerTaskBase::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	if (SynthesisEventArgs.Result->Reason != ResultReason::SynthesizingAudio)
	{
		bLastResultIsValid = AzSpeech::Internal::ProcessSynthesisResult(SynthesisEventArgs.Result);
	}

	LastSynthesizedBuffer.clear();
	LastSynthesizedBuffer = *SynthesisEventArgs.Result->GetAudioData().get();
	LastSynthesizedBuffer.shrink_to_fit();

	if (AzSpeech::Internal::GetPluginSettings()->bEnableRuntimeDebug)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Current audio duration: %s"), *FString(__func__), *FString::FromInt(SynthesisEventArgs.Result->AudioDuration.count()));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Current audio length: %s"), *FString(__func__), *FString::FromInt(SynthesisEventArgs.Result->GetAudioLength()));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Current stream size: %s"), *FString(__func__), *FString::FromInt(SynthesisEventArgs.Result->GetAudioData().get()->size()));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Current reason code: %s"), *FString(__func__), *FString::FromInt(static_cast<int32>(SynthesisEventArgs.Result->Reason)));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: Current result id: %s"), *FString(__func__), *FString(UTF8_TO_TCHAR(SynthesisEventArgs.Result->ResultId.c_str())));
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