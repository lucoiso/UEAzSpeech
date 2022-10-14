// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToVoiceAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

USSMLToVoiceAsync* USSMLToVoiceAsync::SSMLToVoice(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToVoiceAsync* const SSMLToVoiceAsync = NewObject<USSMLToVoiceAsync>();
	SSMLToVoiceAsync->WorldContextObject = WorldContextObject;
	SSMLToVoiceAsync->SSMLString = SSMLString;

	return SSMLToVoiceAsync;
}

void USSMLToVoiceAsync::Activate()
{
	StartAzureTaskWork_Internal();
}

void USSMLToVoiceAsync::StartAzureTaskWork_Internal()
{
	if (SSMLString.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: SSML is empty"), *FString(__func__));
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, this]
	{
		const TFuture<bool> SSMLToVoiceAsyncWork = Async(EAsyncExecution::Thread, [=]() -> bool
		{
			const std::string InSSMLStr = TCHAR_TO_UTF8(*SSMLString);

			return DoAzureTaskWork_Internal(InSSMLStr);
		});

		if (!SSMLToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
			return;
		}

		const bool bOutputValue = SSMLToVoiceAsyncWork.Get();
		AsyncTask(ENamedThreads::GameThread, [=]() { TaskCompleted.Broadcast(bOutputValue); });

		if (bOutputValue)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(FuncName));
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed"), *FString(FuncName));
		}
	});
}

bool USSMLToVoiceAsync::DoAzureTaskWork_Internal(const std::string& InSSML)
{
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer();

	if (!SynthesizerObject)
	{
		return false;
	}

	const auto SynthesisResult = SynthesizerObject->SpeakSsmlAsync(InSSML).get();

	return AzSpeech::Internal::ProcessSynthesizResult(SynthesisResult);
}
