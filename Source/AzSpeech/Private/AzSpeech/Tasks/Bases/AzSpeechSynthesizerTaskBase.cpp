// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeech/Runnables/AzSpeechSynthesisRunnable.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "LogAzSpeech.h"

void UAzSpeechSynthesizerTaskBase::Activate()
{
	ValidateVoiceName();
	
	Super::Activate();
}

const FAzSpeechVisemeData UAzSpeechSynthesizerTaskBase::GetVisemeData() const
{
	if (AzSpeech::Internal::HasEmptyParam(VisemeDataArray))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Viseme data is empty"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
		return FAzSpeechVisemeData();
	}

	return VisemeDataArray.Last();
}

const TArray<FAzSpeechVisemeData> UAzSpeechSynthesizerTaskBase::GetVisemeDataArray() const
{
	return VisemeDataArray;
}

const TArray<uint8> UAzSpeechSynthesizerTaskBase::GetAudioData() const
{
	FScopeLock Lock(&Mutex);

	if (AudioData.empty())
	{
		return TArray<uint8>();
	}

	TArray<uint8> OutputArr;
	for (const uint8_t& i : AudioData)
	{
		OutputArr.Add(static_cast<uint8>(i));
	}

	return OutputArr;
}

const bool UAzSpeechSynthesizerTaskBase::IsLastResultValid() const
{
	return bLastResultIsValid;
}

const FString UAzSpeechSynthesizerTaskBase::GetVoiceName() const
{
	return VoiceName;
}

const FString UAzSpeechSynthesizerTaskBase::GetSynthesisText() const
{
	return SynthesisText;
}

const bool UAzSpeechSynthesizerTaskBase::IsSSMLBased() const
{
	return bIsSSMLBased;
}

void UAzSpeechSynthesizerTaskBase::StartSynthesisWork(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig)
{
	RunnableTask = MakeShared<FAzSpeechSynthesisRunnable>(this, InAudioConfig);

	if (!RunnableTask)
	{
		SetReadyToDestroy();
		return;
	}

	RunnableTask->StartAzSpeechRunnableTask();
}

const bool UAzSpeechSynthesizerTaskBase::CanBroadcastWithReason(const Microsoft::CognitiveServices::Speech::ResultReason& Reason) const
{
	return Reason != Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudio && Reason != Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioStarted;
}

void UAzSpeechSynthesizerTaskBase::OnVisemeReceived(const FAzSpeechVisemeData& VisemeData)
{
	check(IsInGameThread());

	if (!IsTaskStillValid(this))
	{
		return;
	}

	FScopeLock Lock(&Mutex);

	VisemeDataArray.Add(VisemeData);

	if (VisemeReceived.IsBound())
	{
		VisemeReceived.Broadcast(VisemeData);
	}

	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current Viseme Id: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), VisemeData.VisemeID);
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current Viseme Audio Offset: %dms"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), VisemeData.AudioOffsetMilliseconds);
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current Viseme Animation: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *VisemeData.Animation);
}

void UAzSpeechSynthesizerTaskBase::OnSynthesisUpdate(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult)
{
	check(IsInGameThread());

	if (!IsTaskStillValid(this))
	{
		return;
	}

	if (!LastResult)
	{
		SetReadyToDestroy();
		return;
	}

	FScopeLock Lock(&Mutex);

	AudioData = *LastResult->GetAudioData().get();

	bLastResultIsValid = !AudioData.empty();

	if (CanBroadcastWithReason(LastResult->Reason))
	{
		LogSynthesisResultStatus(bLastResultIsValid);
	}

	if (SynthesisUpdated.IsBound())
	{
		SynthesisUpdated.Broadcast();
	}

	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current audio duration: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), LastResult->AudioDuration.count());
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current audio length: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), LastResult->GetAudioLength());
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current stream size: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), LastResult->GetAudioData().get()->size());
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current reason code: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), static_cast<int32>(LastResult->Reason));
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current result id: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *FString(UTF8_TO_TCHAR(LastResult->ResultId.c_str())));
}

void UAzSpeechSynthesizerTaskBase::LogSynthesisResultStatus(const bool bSuccess) const
{
	if (bSuccess)
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Task completed with result: Success"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
	}
	else if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: Task completed with result: Canceled"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
	}
	else
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Task completed with result: Failed"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
	}
}

void UAzSpeechSynthesizerTaskBase::ValidateVoiceName()
{	
	const auto Settings = UAzSpeechSettings::GetAzSpeechKeys();
	if (HasEmptyParameters(VoiceName) || VoiceName.Equals("Default", ESearchCase::IgnoreCase))
	{
		VoiceName = UTF8_TO_TCHAR(Settings.at(3).c_str());
	}
}