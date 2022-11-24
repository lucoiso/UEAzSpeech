// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Bases/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include "Async/Async.h"

void UAzSpeechSynthesizerTaskBase::Activate()
{
	AzSpeech::Internal::GetVoiceName(VoiceName);
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
	
		if (!bAlreadyBroadcastFinal)
		{
			BroadcastFinalResult();
		}

		SynthesizerObject->StopSpeakingAsync().wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));

		if (bNullifySynthesizerObjectOnStop)
		{
			// Free the process handle - Necessary on .wav based tasks
			SynthesizerObject = nullptr;
		}
	});
}

const FAzSpeechVisemeData UAzSpeechSynthesizerTaskBase::GetLastVisemeData() const
{
	return LastVisemeData;
}

const TArray<uint8> UAzSpeechSynthesizerTaskBase::GetLastSynthesizedStream() const
{
	if (LastSynthesizedBuffer.empty())
	{
		return TArray<uint8>();
	}

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

const bool UAzSpeechSynthesizerTaskBase::IsLastResultValid() const
{
	return bLastResultIsValid;
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

void UAzSpeechSynthesizerTaskBase::BroadcastFinalResult()
{
	Super::BroadcastFinalResult();
}

void UAzSpeechSynthesizerTaskBase::EnableVisemeOutput()
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Enabling Viseme"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

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

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Adding extra settings to existing synthesizer object"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

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

void UAzSpeechSynthesizerTaskBase::ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig)
{
	Super::ApplySDKSettings(InConfig);

	InConfig->SetProperty("SpeechSynthesis_KeepConnectionAfterStopping", "false");

	if (IsUsingAutoLanguage())
	{
		return;
	}

	const std::string UsedLang = TCHAR_TO_UTF8(*LanguageId);
	const std::string UsedVoice = TCHAR_TO_UTF8(*VoiceName);

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Using language: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(UTF8_TO_TCHAR(UsedLang.c_str())));
	InConfig->SetSpeechSynthesisLanguage(UsedLang);

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Using voice: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(UTF8_TO_TCHAR(UsedVoice.c_str())));
	InConfig->SetSpeechSynthesisVoiceName(UsedVoice);
}

void UAzSpeechSynthesizerTaskBase::OnVisemeReceived(const Microsoft::CognitiveServices::Speech::SpeechSynthesisVisemeEventArgs& VisemeEventArgs)
{
	const int64 AudioOffsetMs = VisemeEventArgs.AudioOffset / 10000;
	const FString VisemeAnimation_UEStr = UTF8_TO_TCHAR(VisemeEventArgs.Animation.c_str());

	if (AzSpeech::Internal::GetPluginSettings()->bEnableRuntimeDebug)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current Viseme Id: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString::FromInt(VisemeEventArgs.VisemeId));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current Viseme Audio Offset: %sms"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString::FromInt(AudioOffsetMs));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current Viseme Animation: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *VisemeAnimation_UEStr);
	}

	LastVisemeData = FAzSpeechVisemeData(VisemeEventArgs.VisemeId, AudioOffsetMs, VisemeAnimation_UEStr);
	AsyncTask(ENamedThreads::GameThread, [=] { VisemeReceived.Broadcast(LastVisemeData); });
}

void UAzSpeechSynthesizerTaskBase::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	AsyncTask(ENamedThreads::GameThread, [=] { SynthesisUpdated.Broadcast(); });

	if (SynthesisEventArgs.Result->Reason != Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudio)
	{
		bLastResultIsValid = ProcessSynthesisResult(SynthesisEventArgs.Result);
	}

	LastSynthesizedBuffer.clear();
	LastSynthesizedBuffer = *SynthesisEventArgs.Result->GetAudioData().get();
	LastSynthesizedBuffer.shrink_to_fit();

	if (AzSpeech::Internal::GetPluginSettings()->bEnableRuntimeDebug)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current audio duration: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString::FromInt(SynthesisEventArgs.Result->AudioDuration.count()));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current audio length: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString::FromInt(SynthesisEventArgs.Result->GetAudioLength()));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current stream size: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString::FromInt(SynthesisEventArgs.Result->GetAudioData().get()->size()));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current reason code: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString::FromInt(static_cast<int32>(SynthesisEventArgs.Result->Reason)));
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Current result id: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(UTF8_TO_TCHAR(SynthesisEventArgs.Result->ResultId.c_str())));
	}
}

bool UAzSpeechSynthesizerTaskBase::InitializeSynthesizer(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig)
{
	if (!AzSpeech::Internal::CheckAzSpeechSettings())
	{
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Initializing synthesizer object"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

	const auto SpeechConfig = UAzSpeechTaskBase::CreateSpeechConfig();

	if (!SpeechConfig)
	{
		return false;
	}

	ApplySDKSettings(SpeechConfig);

	if (IsUsingAutoLanguage())
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Initializing auto language detection"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

		SynthesizerObject = Microsoft::CognitiveServices::Speech::SpeechSynthesizer::FromConfig(SpeechConfig, Microsoft::CognitiveServices::Speech::AutoDetectSourceLanguageConfig::FromOpenRange(), InAudioConfig);
	}
	else 
	{
		SynthesizerObject = Microsoft::CognitiveServices::Speech::SpeechSynthesizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	ApplyExtraSettings();

	return true;
}

void UAzSpeechSynthesizerTaskBase::StartSynthesisWork()
{
	if (!SynthesizerObject)
	{
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Starting synthesis"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));

	if (AzSpeech::Internal::GetPluginSettings()->bEnableRuntimeDebug)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Using text: %s"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *SynthesisText);
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]
	{
		const std::string SynthesisStr = TCHAR_TO_UTF8(*SynthesisText);
		std::future<std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>> Future;
		if (bIsSSMLBased)
		{
			Future = SynthesizerObject->SpeakSsmlAsync(SynthesisStr);
		}
		else
		{
			Future = SynthesizerObject->SpeakTextAsync(SynthesisStr);
		}

		Future.wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
	});

	
}

void UAzSpeechSynthesizerTaskBase::OutputSynthesisResult(const bool bSuccess) const
{
	if (bSuccess)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Task completed with result: Success"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
	}
	else if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Task completed with result: Canceled"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
	}
	else
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Task completed with result: Failed"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
	}
}

const bool UAzSpeechSynthesizerTaskBase::ProcessSynthesisResult(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& Result) const
{
	switch (Result->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudio:
			UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Task running. Reason: SynthesizingAudio"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioCompleted:
			UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Task completed. Reason: SynthesizingAudioCompleted"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioStarted:
			UE_LOG(LogAzSpeech, Display, TEXT("%s: AzSpeech Task: %s (%s); Task started. Reason: SynthesizingAudioStarted"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
			return true;

		default:
			break;
	}

	if (Result->Reason == Microsoft::CognitiveServices::Speech::ResultReason::Canceled)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Task failed. Reason: Canceled"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::SpeechSynthesisCancellationDetails::FromResult(Result);

		UE_LOG(LogAzSpeech, Error, TEXT("%s: AzSpeech Task: %s (%s); Cancellation Reason: %s"), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

		if (CancellationDetails->Reason == Microsoft::CognitiveServices::Speech::CancellationReason::Error)
		{
			ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
		}

		return false;
	}

	UE_LOG(LogAzSpeech, Warning, TEXT("%s: AzSpeech Task: %s (%s); Ended with undefined reason"), *FString(__func__), *TaskName.ToString(), *FString::FromInt(GetUniqueID()));
	return false;
}

const bool UAzSpeechSynthesizerTaskBase::CanBroadcastWithReason(const Microsoft::CognitiveServices::Speech::ResultReason& Reason) const
{
	return Reason != Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudio && Reason != Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioStarted;
}
