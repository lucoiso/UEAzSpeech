// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeech/Runnables/AzSpeechSynthesisRunnable.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechSynthesizerTaskBase)
#endif

void UAzSpeechSynthesizerTaskBase::Activate()
{
	ValidateVoiceName();
	
	Super::Activate();
}

const FAzSpeechVisemeData& UAzSpeechSynthesizerTaskBase::GetLastVisemeData() const
{
	FScopeLock Lock(&Mutex);

	if (AzSpeech::Internal::HasEmptyParam(VisemeDataArray))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Viseme data is empty"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
		return FAzSpeechVisemeData();
	}

	return VisemeDataArray.Last();
}

const TArray<FAzSpeechVisemeData> UAzSpeechSynthesizerTaskBase::GetVisemeDataArray() const
{
	FScopeLock Lock(&Mutex);

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
	OutputArr.Append(reinterpret_cast<const uint8*>(AudioData.data()), AudioData.size());

	return OutputArr;
}

const bool UAzSpeechSynthesizerTaskBase::IsLastResultValid() const
{
	FScopeLock Lock(&Mutex);

	return bLastResultIsValid;
}

const FString UAzSpeechSynthesizerTaskBase::GetVoiceName() const
{
	FScopeLock Lock(&Mutex);

	return VoiceName;
}

const FString UAzSpeechSynthesizerTaskBase::GetSynthesisText() const
{
	FScopeLock Lock(&Mutex);

	return SynthesisText;
}

const bool UAzSpeechSynthesizerTaskBase::IsSSMLBased() const
{
	FScopeLock Lock(&Mutex);

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

void UAzSpeechSynthesizerTaskBase::OnVisemeReceived(const FAzSpeechVisemeData& VisemeData)
{
	FScopeLock Lock(&Mutex);

	check(IsInGameThread());

	VisemeDataArray.Add(VisemeData);
	VisemeReceived.Broadcast(VisemeData);

	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current Viseme Id: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), VisemeData.VisemeID);
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current Viseme Audio Offset: %dms"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), VisemeData.AudioOffsetMilliseconds);
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current Viseme Animation: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *VisemeData.Animation);
}

void UAzSpeechSynthesizerTaskBase::OnSynthesisUpdate(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult)
{
	FScopeLock Lock(&Mutex);

	check(IsInGameThread());

	AudioData = *LastResult->GetAudioData().get();
	bLastResultIsValid = !AudioData.empty();

	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current audio duration: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), LastResult->AudioDuration.count());
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current audio length: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), LastResult->GetAudioLength());
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current stream size: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), LastResult->GetAudioData().get()->size());
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current reason code: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), static_cast<int32>(LastResult->Reason));
	UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Current result id: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), UTF8_TO_TCHAR(LastResult->ResultId.c_str()));

	SynthesisUpdated.Broadcast();
}

void UAzSpeechSynthesizerTaskBase::ValidateVoiceName()
{
	FScopeLock Lock(&Mutex);

	if (bIsSSMLBased)
	{
		return;
	}

	const auto Settings = UAzSpeechSettings::GetAzSpeechKeys();
	if (AzSpeech::Internal::HasEmptyParam(VoiceName) || VoiceName.Equals("Default", ESearchCase::IgnoreCase))
	{
		VoiceName = UTF8_TO_TCHAR(Settings.at(3).c_str());
	}
}