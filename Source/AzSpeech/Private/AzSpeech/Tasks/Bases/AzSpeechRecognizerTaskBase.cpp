// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechRecognizerTaskBase.h"
#include "AzSpeech/Runnables/AzSpeechRecognitionRunnable.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>

#if !UE_BUILD_SHIPPING
#include <Engine/Engine.h>
#endif

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechRecognizerTaskBase)
#endif

const FString UAzSpeechRecognizerTaskBase::GetRecognizedString() const
{
	FScopeLock Lock(&Mutex);

	if (RecognizedText.empty())
	{
		return FString();
	}

	return UTF8_TO_TCHAR(RecognizedText.c_str());
}

const int32 UAzSpeechRecognizerTaskBase::GetRecognitionLatency() const
{
	FScopeLock Lock(&Mutex);

	return RecognitionLatency;
}

void UAzSpeechRecognizerTaskBase::StartRecognitionWork(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig)
{
	RunnableTask = MakeShared<FAzSpeechRecognitionRunnable>(this, InAudioConfig);

	if (!RunnableTask)
	{
		SetReadyToDestroy();
		return;
	}
	
	RunnableTask->StartAzSpeechRunnableTask();
}

void UAzSpeechRecognizerTaskBase::BroadcastFinalResult()
{
	if (!UAzSpeechTaskStatus::IsTaskActive(this))
	{
		return;
	}

	Super::BroadcastFinalResult();

	AsyncTask(ENamedThreads::GameThread,
		[this]
		{
			RecognitionCompleted.Broadcast(GetRecognizedString());
		}
	);
}

void UAzSpeechRecognizerTaskBase::OnRecognitionUpdated(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognitionResult>& LastResult)
{
	check(IsInGameThread());

	FScopeLock Lock(&Mutex);

	RecognizedText = LastResult->Text;
	RecognitionLatency = GetProperty<int32>(LastResult, Microsoft::CognitiveServices::Speech::PropertyId::SpeechServiceResponse_RecognitionLatencyMs);

	RecognitionUpdated.Broadcast(GetRecognizedString());

	if (UAzSpeechSettings::Get()->bEnableDebuggingLogs || UAzSpeechSettings::Get()->bEnableDebuggingPrints)
	{
		const auto TicksToMs = [](const auto& Ticks)
		{
			return static_cast<uint64>(Ticks / 10000u);
		};

		const FStringFormatOrderedArguments Arguments {
			TaskName.ToString(),
			GetUniqueID(),
			FString(__func__),
			UTF8_TO_TCHAR(LastResult->Text.c_str()),
			TicksToMs(LastResult->Duration()),
			TicksToMs(LastResult->Offset()),
			static_cast<int32>(LastResult->Reason),
			UTF8_TO_TCHAR(LastResult->ResultId.c_str()),
			RecognitionLatency
		};

		const FString MountedDebuggingInfo = FString::Format(TEXT("Task: {0} ({1});\n\tFunction: {2};\n\tRecognized text: {3}\n\tDuration: {4}ms\n\tOffset: {5}ms\n\tReason code: {6}\n\tResult ID: {7}\n\tRecognition latency: {8}ms"), Arguments);

		UE_LOG(LogAzSpeech_Debugging, Display, TEXT("%s"), *MountedDebuggingInfo);

#if !UE_BUILD_SHIPPING
		if (UAzSpeechSettings::Get()->bEnableDebuggingPrints)
		{
			GEngine->AddOnScreenDebugMessage(static_cast<int32>(GetUniqueID()), 5.f, FColor::Yellow, MountedDebuggingInfo);
		}
#endif
	}
}