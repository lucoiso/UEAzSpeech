// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToStreamAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static std::vector<uint8_t> DoTextToStreamWork(const std::string& InStr, const std::string& InLanguageID, const std::string& InVoiceName)
		{
			const auto AudioConfig = AudioConfig::FromStreamOutput(AudioOutputStream::CreatePullStream());
			const auto Synthesizer = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig, InLanguageID, InVoiceName);

			if (Synthesizer == nullptr)
			{
				return std::vector<uint8_t>();
			}

			if (const auto SynthesisResult = Synthesizer->SpeakTextAsync(InStr).get(); 
				AzSpeech::Internal::ProcessAzSpeechResult(SynthesisResult->Reason))
			{
				return *SynthesisResult->GetAudioData().get();
			}

			return std::vector<uint8_t>();
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncTextToStream(const FString& InStr, const FString& InVoiceName, const FString& InLanguageID, const FTextToStreamDelegate& InDelegate)
		{
			if (InStr.IsEmpty() || InVoiceName.IsEmpty() || InLanguageID.IsEmpty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, InStr, InVoiceName, InLanguageID, InDelegate]
			{
				const TFuture<std::vector<uint8_t>> TextToStreamAsyncWork = Async(EAsyncExecution::Thread, [=]() -> std::vector<uint8_t>
				{
					const std::string InConvertStr = TCHAR_TO_UTF8(*InStr);
					const std::string InLanguageIDStr = TCHAR_TO_UTF8(*InLanguageID);
					const std::string InNameIDStr = TCHAR_TO_UTF8(*InVoiceName);

					return Standard_Cpp::DoTextToStreamWork(InConvertStr, InLanguageIDStr, InNameIDStr);
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

				InDelegate.Broadcast(OutputArr);

				if (bOutputValue)
				{
					UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(FuncName));
				}
				else
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Error"), *FString(FuncName));
				}
			});
		}
	}
}

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
	AzSpeechWrapper::Unreal_Cpp::AsyncTextToStream(TextToConvert, VoiceName, LanguageID, TaskCompleted);
}
