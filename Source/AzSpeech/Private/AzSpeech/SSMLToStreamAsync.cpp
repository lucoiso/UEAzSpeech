// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToStreamAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

USSMLToStreamAsync* USSMLToStreamAsync::SSMLToStream(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToStreamAsync* const NewAsyncTask = NewObject<USSMLToStreamAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SSMLString = SSMLString;

	return NewAsyncTask;
}

void USSMLToStreamAsync::Activate()
{
	Super::Activate();
}

bool USSMLToStreamAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (SSMLString.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: SSML is empty"), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, this]
	{
		const TFuture<std::vector<uint8_t>> SSMLToStreamAsyncWork = Async(EAsyncExecution::Thread, [=]() -> std::vector<uint8_t>
		{
			const std::string InSSMLStr = TCHAR_TO_UTF8(*SSMLString);

			return DoAzureTaskWork_Internal(InSSMLStr);
		});

		if (!SSMLToStreamAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
		}

		const std::vector<uint8_t> Result = SSMLToStreamAsyncWork.Get();
		const bool bOutputValue = !Result.empty();

		TArray<uint8> OutputArr;
		for (const uint8_t& i : Result)
		{
			OutputArr.Add(static_cast<uint8>(i));
		}

		AsyncTask(ENamedThreads::GameThread, [=]() { TaskCompleted.Broadcast(OutputArr); });

		if (bOutputValue)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(FuncName));
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed"), *FString(FuncName));
		}
	});

	return true;
}

std::vector<uint8_t> USSMLToStreamAsync::DoAzureTaskWork_Internal(const std::string& InSSML)
{
	const auto AudioConfig = AudioConfig::FromStreamOutput(AudioOutputStream::CreatePullStream());
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig);

	if (!SynthesizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Failed to proceed with task: SynthesizerObject is null"), *FString(__func__));
		return std::vector<uint8_t>();
	}

	if (const auto SpeechSynthesisResult = SynthesizerObject->SpeakSsmlAsync(InSSML).get();
		AzSpeech::Internal::ProcessSynthesizResult(SpeechSynthesisResult))
	{
		return *SpeechSynthesisResult->GetAudioData().get();
	}

	return std::vector<uint8_t>();
}