// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeech/Runnables/AzSpeechSynthesisRunnable.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>

#if !UE_BUILD_SHIPPING
#include <Engine.h>
#endif

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechSynthesizerTaskBase)
#endif

const FAzSpeechVisemeData UAzSpeechSynthesizerTaskBase::GetLastVisemeData() const
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

const FAzSpeechAnimationData UAzSpeechSynthesizerTaskBase::GetLastExtractedAnimationData() const
{
	FScopeLock Lock(&Mutex);

	return UAzSpeechHelper::ExtractAnimationDataFromVisemeData(GetLastVisemeData());
}

const TArray<FAzSpeechAnimationData> UAzSpeechSynthesizerTaskBase::GetExtractedAnimationDataArray() const
{
	FScopeLock Lock(&Mutex);

	return UAzSpeechHelper::ExtractAnimationDataFromVisemeDataArray(GetVisemeDataArray());
}

const bool UAzSpeechSynthesizerTaskBase::IsLastResultValid() const
{
	FScopeLock Lock(&Mutex);

	return bLastResultIsValid;
}

const FString UAzSpeechSynthesizerTaskBase::GetSynthesisText() const
{
	return SynthesisText;
}

const bool UAzSpeechSynthesizerTaskBase::IsSSMLBased() const
{
	return bIsSSMLBased;
}

const int32 UAzSpeechSynthesizerTaskBase::GetConnectionLatency() const
{
	FScopeLock Lock(&Mutex);

	return ConnectionLatency;
}

const int32 UAzSpeechSynthesizerTaskBase::GetFinishLatency() const
{
	FScopeLock Lock(&Mutex);

	return FinishLatency;
}

const int32 UAzSpeechSynthesizerTaskBase::GetFirstByteLatency() const
{
	FScopeLock Lock(&Mutex);

	return FirstByteLatency;
}

const int32 UAzSpeechSynthesizerTaskBase::GetNetworkLatency() const
{
	FScopeLock Lock(&Mutex);

	return NetworkLatency;
}

const int32 UAzSpeechSynthesizerTaskBase::GetServiceLatency() const
{
	FScopeLock Lock(&Mutex);

	return ServiceLatency;
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

	if (UAzSpeechSettings::Get()->bEnableDebuggingLogs || UAzSpeechSettings::Get()->bEnableDebuggingPrints)
	{
		const FStringFormatOrderedArguments Arguments{
			TaskName.ToString(),
			GetUniqueID(),
			FString(__func__),
			VisemeData.VisemeID,
			VisemeData.AudioOffsetMilliseconds,
			VisemeData.Animation
		};

		const FString MountedDebuggingInfo = FString::Format(TEXT("Task: {0} ({1}); Function: {2}; Message:\n\tViseme ID: {3}\n\tViseme audio offset: {4}ms\n\tViseme animation: {5}"), Arguments);

		UE_LOG(LogAzSpeech_Debugging, Display, TEXT("%s"), *MountedDebuggingInfo);

#if !UE_BUILD_SHIPPING
		if (UAzSpeechSettings::Get()->bEnableDebuggingPrints)
		{
			GEngine->AddOnScreenDebugMessage(static_cast<int32>(GetUniqueID()), 5.f, FColor::Yellow, MountedDebuggingInfo);
		}
#endif
	}
	
	VisemeDataArray.Add(VisemeData);

	AsyncTask(ENamedThreads::GameThread,
		[this, VisemeData]
		{
			VisemeReceived.Broadcast(VisemeData);
		}
	);
}

void UAzSpeechSynthesizerTaskBase::OnSynthesisUpdate(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult)
{
	FScopeLock Lock(&Mutex);

	ConnectionLatency = static_cast<int32>(std::stoi(LastResult->Properties.GetProperty(Microsoft::CognitiveServices::Speech::PropertyId::SpeechServiceResponse_SynthesisConnectionLatencyMs)));
	FinishLatency = static_cast<int32>(std::stoi(LastResult->Properties.GetProperty(Microsoft::CognitiveServices::Speech::PropertyId::SpeechServiceResponse_SynthesisFinishLatencyMs)));
	FirstByteLatency = static_cast<int32>(std::stoi(LastResult->Properties.GetProperty(Microsoft::CognitiveServices::Speech::PropertyId::SpeechServiceResponse_SynthesisFirstByteLatencyMs)));
	NetworkLatency = static_cast<int32>(std::stoi(LastResult->Properties.GetProperty(Microsoft::CognitiveServices::Speech::PropertyId::SpeechServiceResponse_SynthesisNetworkLatencyMs)));
	ServiceLatency = static_cast<int32>(std::stoi(LastResult->Properties.GetProperty(Microsoft::CognitiveServices::Speech::PropertyId::SpeechServiceResponse_SynthesisServiceLatencyMs)));
	
	if (UAzSpeechSettings::Get()->bEnableDebuggingLogs || UAzSpeechSettings::Get()->bEnableDebuggingPrints)
	{
		const FStringFormatOrderedArguments Arguments{
			TaskName.ToString(),
			GetUniqueID(),
			FString(__func__),
			static_cast<int64>(LastResult->AudioDuration.count()),
			static_cast<uint32>(LastResult->GetAudioLength()),
			static_cast<uint32>(LastResult->GetAudioData().get()->size()),
			static_cast<int32>(LastResult->Reason),
			UTF8_TO_TCHAR(LastResult->ResultId.c_str()),
			ConnectionLatency,
			FinishLatency,
			FirstByteLatency,
			NetworkLatency,
			ServiceLatency
		};

		const FString MountedDebuggingInfo = FString::Format(TEXT("Task: {0} ({1}); Function: {2}; Message:\n\tAudio duration: {3}\n\tAudio lenght: {4}\n\tStream size: {5}\n\tReason code: {6}\n\tResult ID: {7}\n\tConnection latency: {8}ms\n\tFinish latency: {9}ms\n\tFirst byte latency: {10}ms\n\tNetwork latency: {11}ms\n\tService latency: {12}ms"), Arguments);

		UE_LOG(LogAzSpeech_Debugging, Display, TEXT("%s"), *MountedDebuggingInfo);

#if !UE_BUILD_SHIPPING
		if (UAzSpeechSettings::Get()->bEnableDebuggingPrints)
		{
			GEngine->AddOnScreenDebugMessage(static_cast<int32>(GetUniqueID()), 5.f, FColor::Yellow, MountedDebuggingInfo);
		}
#endif
	}

	AudioData = *LastResult->GetAudioData().get();
	bLastResultIsValid = !AudioData.empty();

	AsyncTask(ENamedThreads::GameThread,
		[this]
		{
			SynthesisUpdated.Broadcast();
		}
	);
}
