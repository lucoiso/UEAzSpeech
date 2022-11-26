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
		SetReadyToDestroy();
		return;
	}

	if (bCanBroadcastFinal)
	{
		BroadcastFinalResult();
	}
	
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		if (!Mutex.TryLock())
		{
			return;
		}

		SynthesizerObject->StopSpeakingAsync().wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));

		if (bNullifySynthesizerObjectOnStop)
		{
			// Free the process handle - Necessary on .wav based tasks
			SynthesizerObject = nullptr;
		}
	});

	if (IsValid(this))
	{
		SetReadyToDestroy();
	}
}

const FAzSpeechVisemeData UAzSpeechSynthesizerTaskBase::GetLastVisemeData() const
{
	if (AzSpeech::Internal::HasEmptyParam(VisemeDataArray))
	{
		return FAzSpeechVisemeData();
	}

	return VisemeDataArray.Last();
}

const TArray<FAzSpeechVisemeData> UAzSpeechSynthesizerTaskBase::GetVisemeDataArray() const
{
	return VisemeDataArray;
}

const TArray<uint8> UAzSpeechSynthesizerTaskBase::GetLastSynthesizedAudioData() const
{
	if (!LastSynthesisResult || !LastSynthesisResult->GetAudioData())
	{
		return TArray<uint8>();
	}

	const std::vector<uint8_t> LastAudioBuffer = *LastSynthesisResult->GetAudioData().get();

	if (LastAudioBuffer.empty())
	{
		return TArray<uint8>();
	}

	TArray<uint8> OutputArr;
	for (const uint8_t& i : LastAudioBuffer)
	{
		OutputArr.Add(static_cast<uint8>(i));
	}

	return OutputArr;
}

const bool UAzSpeechSynthesizerTaskBase::IsLastVisemeDataValid() const
{
	return GetLastVisemeData().IsValid();
}

const bool UAzSpeechSynthesizerTaskBase::IsLastResultValid() const
{
	return bLastResultIsValid;
}

void UAzSpeechSynthesizerTaskBase::ClearBindings()
{
	Super::ClearBindings();

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
	VisemeDataArray.Empty();
	
	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Enabling Viseme"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

	if (VisemeReceived.IsBound())
	{
		SynthesizerObject->VisemeReceived.Connect([this](const Microsoft::CognitiveServices::Speech::SpeechSynthesisVisemeEventArgs& VisemeEventArgs)
		{
			FAzSpeechVisemeData LastVisemeData;
			LastVisemeData.VisemeID = VisemeEventArgs.VisemeId;
			LastVisemeData.AudioOffsetMilliseconds = VisemeEventArgs.AudioOffset / 10000;
			LastVisemeData.Animation = UTF8_TO_TCHAR(VisemeEventArgs.Animation.c_str());

			AsyncTask(ENamedThreads::GameThread, [this, LastVisemeData] { OnVisemeReceived(LastVisemeData); });
		});
	}
}

void UAzSpeechSynthesizerTaskBase::ConnectTaskSignals()
{
	Super::ConnectTaskSignals();

	if (!SynthesizerObject)
	{
		StopAzSpeechTask();
		return;
	}

	const auto SynthesisUpdate_Lambda = [this](const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
	{
		FScopeLock Lock(&Mutex);

		LastSynthesisResult = SynthesisEventArgs.Result;
		AsyncTask(ENamedThreads::GameThread, [this] { OnSynthesisUpdate(); });
	};

	SynthesizerObject->SynthesisStarted.Connect(SynthesisUpdate_Lambda);
	SynthesizerObject->Synthesizing.Connect(SynthesisUpdate_Lambda);
	SynthesizerObject->SynthesisCompleted.Connect(SynthesisUpdate_Lambda);
	SynthesizerObject->SynthesisCanceled.Connect(SynthesisUpdate_Lambda);

	if (AzSpeech::Internal::GetPluginSettings()->bEnableViseme)
	{
		EnableVisemeOutput();
	}
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

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Using language: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString(UTF8_TO_TCHAR(UsedLang.c_str())));
	InConfig->SetSpeechSynthesisLanguage(UsedLang);

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Using voice: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString(UTF8_TO_TCHAR(UsedVoice.c_str())));
	InConfig->SetSpeechSynthesisVoiceName(UsedVoice);
}

void UAzSpeechSynthesizerTaskBase::OnVisemeReceived(const FAzSpeechVisemeData& VisemeData)
{
	check(IsInGameThread());

	FScopeLock Lock(&Mutex);
	
	VisemeDataArray.Add(VisemeData);
	VisemeReceived.Broadcast(VisemeData);

	if (AzSpeech::Internal::GetPluginSettings()->bEnableRuntimeDebug)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current Viseme Id: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(VisemeData.VisemeID));
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current Viseme Audio Offset: %sms"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(VisemeData.AudioOffsetMilliseconds));
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current Viseme Animation: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *VisemeData.Animation);
	}
}

void UAzSpeechSynthesizerTaskBase::OnSynthesisUpdate()
{
	check(IsInGameThread());

	if (!LastSynthesisResult)
	{
		return;
	}

	FScopeLock Lock(&Mutex);

	if (LastSynthesisResult->Reason != Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudio)
	{
		bLastResultIsValid = ProcessLastSynthesisResult();
	}

	SynthesisUpdated.Broadcast();

	if (AzSpeech::Internal::GetPluginSettings()->bEnableRuntimeDebug)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current audio duration: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(LastSynthesisResult->AudioDuration.count()));
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current audio length: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(LastSynthesisResult->GetAudioLength()));
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current stream size: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(LastSynthesisResult->GetAudioData().get()->size()));
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current reason code: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(static_cast<int32>(LastSynthesisResult->Reason)));
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Current result id: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString(UTF8_TO_TCHAR(LastSynthesisResult->ResultId.c_str())));
	}
}

bool UAzSpeechSynthesizerTaskBase::InitializeSynthesizer(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig)
{
	if (!AzSpeech::Internal::CheckAzSpeechSettings())
	{
		StopAzSpeechTask();
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Initializing synthesizer object"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

	const auto SpeechConfig = UAzSpeechTaskBase::CreateSpeechConfig();

	if (!SpeechConfig)
	{
		StopAzSpeechTask();
		return false;
	}

	ApplySDKSettings(SpeechConfig);

	if (IsUsingAutoLanguage())
	{
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Initializing auto language detection"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

		SynthesizerObject = Microsoft::CognitiveServices::Speech::SpeechSynthesizer::FromConfig(SpeechConfig, Microsoft::CognitiveServices::Speech::AutoDetectSourceLanguageConfig::FromOpenRange(), InAudioConfig);
	}
	else
	{
		SynthesizerObject = Microsoft::CognitiveServices::Speech::SpeechSynthesizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	ConnectTaskSignals();

	return true;
}

void UAzSpeechSynthesizerTaskBase::StartSynthesisWork()
{
	if (!SynthesizerObject)
	{
		StopAzSpeechTask();
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Starting synthesis"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));

	if (AzSpeech::Internal::GetPluginSettings()->bEnableRuntimeDebug)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Using text: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *SynthesisText);
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
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

void UAzSpeechSynthesizerTaskBase::LogSynthesisResultStatus(const bool bSuccess) const
{
	if (bSuccess)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Task completed with result: Success"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	}
	else if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Task completed with result: Canceled"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	}
	else
	{
		UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%s); Function: %s; Message: Task completed with result: Failed"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	}
}

const bool UAzSpeechSynthesizerTaskBase::ProcessLastSynthesisResult() const
{
	switch (LastSynthesisResult->Reason)
	{
		case Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudio:
			UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Task running. Reason: SynthesizingAudio"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioCompleted:
			UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Task completed. Reason: SynthesizingAudioCompleted"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return true;

		case Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioStarted:
			UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: Task started. Reason: SynthesizingAudioStarted"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
			return true;

		default:
			break;
	}

	if (LastSynthesisResult->Reason == Microsoft::CognitiveServices::Speech::ResultReason::Canceled)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%s); Function: %s; Message: Task failed. Reason: Canceled"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
		const auto CancellationDetails = Microsoft::CognitiveServices::Speech::SpeechSynthesisCancellationDetails::FromResult(LastSynthesisResult);

		UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%s); Function: %s; Message: Cancellation Reason: %s"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

		if (CancellationDetails->Reason == Microsoft::CognitiveServices::Speech::CancellationReason::Error)
		{
			ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
		}

		return false;
	}

	UE_LOG(LogAzSpeech, Warning, TEXT("Task: %s (%s); Function: %s; Message: Ended with undefined reason"), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__));
	return false;
}

const bool UAzSpeechSynthesizerTaskBase::CanBroadcastWithReason(const Microsoft::CognitiveServices::Speech::ResultReason& Reason) const
{
	return Reason != Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudio && Reason != Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioStarted;
}
