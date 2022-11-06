// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToStreamAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

UTextToStreamAsync* UTextToStreamAsync::TextToStream(const UObject* WorldContextObject, const FString& TextToConvert, const FString& VoiceName, const FString& LanguageId)
{
	UTextToStreamAsync* const NewAsyncTask = NewObject<UTextToStreamAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->TextToConvert = TextToConvert;
	NewAsyncTask->VoiceName = AzSpeech::Internal::GetVoiceName(VoiceName);
	NewAsyncTask->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);

	return NewAsyncTask;
}

void UTextToStreamAsync::Activate()
{
	Super::Activate();
}

bool UTextToStreamAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}
	
	if (TextToConvert.IsEmpty() || VoiceName.IsEmpty() || LanguageID.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, this]
	{
		const TFuture<std::vector<uint8_t>> TextToStreamAsyncWork = Async(EAsyncExecution::Thread, [=]() -> std::vector<uint8_t>
		{
			const std::string InConvertStr = TCHAR_TO_UTF8(*TextToConvert);
			const std::string InLanguageIDStr = TCHAR_TO_UTF8(*LanguageID);
			const std::string InNameIDStr = TCHAR_TO_UTF8(*VoiceName);

			return DoAzureTaskWork_Internal(InConvertStr, InLanguageIDStr, InNameIDStr);
		});

		if (!TextToStreamAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
			return;
		}

		const std::vector<uint8_t> Result = TextToStreamAsyncWork.Get();
		const bool bOutputValue = !Result.empty();

		TArray<uint8> OutputArr;
		for (const uint8_t& i : Result)
		{
			OutputArr.Add(static_cast<uint8>(i));
		}

		AsyncTask(ENamedThreads::GameThread, [=]() { if (CanBroadcast()) { TaskCompleted.Broadcast(OutputArr); } });

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

std::vector<uint8_t> UTextToStreamAsync::DoAzureTaskWork_Internal(const std::string& InStr, const std::string& InLanguageID, const std::string& InVoiceName)
{
	const auto AudioConfig = AudioConfig::FromStreamOutput(AudioOutputStream::CreatePullStream());
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig, InLanguageID, InVoiceName);

	if (!SynthesizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Failed to proceed with task: SynthesizerObject is null"), *FString(__func__));
		return std::vector<uint8_t>();
	}

	CheckAndAddViseme();

	if (const auto SynthesisResult = SynthesizerObject->SpeakTextAsync(InStr).get();
		AzSpeech::Internal::ProcessSynthesizResult(SynthesisResult))
	{
		return *SynthesisResult->GetAudioData().get();
	}

	return std::vector<uint8_t>();
}
