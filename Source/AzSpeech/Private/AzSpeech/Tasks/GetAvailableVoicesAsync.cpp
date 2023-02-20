// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/GetAvailableVoicesAsync.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_speech_synthesizer.h>
THIRD_PARTY_INCLUDES_END

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(GetAvailableVoicesAsync)
#endif

UGetAvailableVoicesAsync* UGetAvailableVoicesAsync::GetAvailableVoicesAsync(UObject* WorldContextObject, const FString& Locale)
{
	UGetAvailableVoicesAsync* const NewAsyncTask = NewObject<UGetAvailableVoicesAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->TaskName = *FString(__func__);
	NewAsyncTask->Locale = Locale;
	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}

void UGetAvailableVoicesAsync::Activate()
{
	Super::Activate();

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Activating task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		const TArray<FString> TaskResult = GetAvailableVoices();
		AsyncTask(ENamedThreads::GameThread, [this, TaskResult] { BroadcastResult(TaskResult); });
	});
}

void UGetAvailableVoicesAsync::SetReadyToDestroy()
{
	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Setting task as Ready to Destroy"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

	Super::SetReadyToDestroy();
}

void UGetAvailableVoicesAsync::BroadcastResult(const TArray<FString>& Result)
{
	check(IsInGameThread());

	UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Task completed. Broadcasting result num: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), Result.Num());

	if (Result.Num() <= 0)
	{
		Fail.Broadcast();
	}
	else
	{
		Success.Broadcast(Result);
	}

	if (Fail.IsBound())
	{
		Fail.Clear();
	}

	if (Success.IsBound())
	{
		Success.Clear();
	}

	SetReadyToDestroy();
}

const TArray<FString> UGetAvailableVoicesAsync::GetAvailableVoices() const
{
	TArray<FString> Output;

	const auto Settings = UAzSpeechSettings::GetAzSpeechKeys();
	if (const auto SpeechConfig = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1)))
	{
		if (std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesizer> SpeechSynthesizer = Microsoft::CognitiveServices::Speech::SpeechSynthesizer::FromConfig(SpeechConfig))
		{
			const auto SynthesisVoices = SpeechSynthesizer->GetVoicesAsync(TCHAR_TO_UTF8(*Locale)).get();
			for (const auto& Voice : SynthesisVoices->Voices)
			{
				UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Voice Name: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), UTF8_TO_TCHAR(Voice->Name.c_str()));
				UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Voice Short Name: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), UTF8_TO_TCHAR(Voice->ShortName.c_str()));
				UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Voice Local Name: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), UTF8_TO_TCHAR(Voice->LocalName.c_str()));
				UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Voice Path: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), UTF8_TO_TCHAR(Voice->VoicePath.c_str()));
				UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Voice Locale: %s"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), UTF8_TO_TCHAR(Voice->Locale.c_str()));
				UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Voice Gender: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), Voice->Gender);
				UE_LOG(LogAzSpeech_Debugging, Display, TEXT("Task: %s (%d); Function: %s; Message: Voice Type: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), Voice->VoiceType);

				Output.Add(UTF8_TO_TCHAR(Voice->ShortName.c_str()));
			}
		}
	}

	return Output;
}
