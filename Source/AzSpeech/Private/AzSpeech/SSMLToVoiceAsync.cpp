// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToVoiceAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

USSMLToVoiceAsync* USSMLToVoiceAsync::SSMLToVoice(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToVoiceAsync* const NewAsyncTask = NewObject<USSMLToVoiceAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SSMLString = SSMLString;

	return NewAsyncTask;
}

void USSMLToVoiceAsync::Activate()
{
	Super::Activate();
}

bool USSMLToVoiceAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}
	
	if (SSMLString.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: SSML is empty"), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: Initializing task"), *FString(__func__));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, this]
	{
		const TFuture<bool> SSMLToVoiceAsyncWork = Async(EAsyncExecution::Thread, [=]() -> bool
		{
			const std::string InSSMLStr = TCHAR_TO_UTF8(*SSMLString);

			return DoAzureTaskWork_Internal(InSSMLStr);
		});

		if (!SSMLToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Task timed out"), *FString(FuncName));
		}

		const bool bOutputValue = SSMLToVoiceAsyncWork.Get();
		if (CanBroadcast())
		{
			AsyncTask(ENamedThreads::GameThread, [=]() { SynthesisCompleted.Broadcast(bOutputValue); });
		}		

		if (bOutputValue)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("%s: Result: Success"), *FString(FuncName));
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Result: Failed"), *FString(FuncName));
		}
	});

	return true;
}

bool USSMLToVoiceAsync::DoAzureTaskWork_Internal(const std::string& InSSML)
{
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer();

	if (!SynthesizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Failed to proceed with task: SynthesizerObject is null"), *FString(__func__));
		return false;
	}

	EnableVisemeOutput();

	const auto SynthesisResult = SynthesizerObject->StartSpeakingSsmlAsync(InSSML).get();

	return AzSpeech::Internal::ProcessSynthesisResult(SynthesisResult);
}
