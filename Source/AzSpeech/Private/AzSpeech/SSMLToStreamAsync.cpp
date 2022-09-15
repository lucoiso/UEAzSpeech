// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToStreamAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static std::vector<uint8_t> DoSSMLToStreamWork(const std::string& InSSML)
		{
			const auto AudioConfig = AudioConfig::FromStreamOutput(AudioOutputStream::CreatePullStream());
			const auto Synthesizer = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig);

			if (Synthesizer == nullptr)
			{
				return std::vector<uint8_t>();
			}

			if (const auto SpeechSynthesisResult = Synthesizer->SpeakSsmlAsync(InSSML).get();
				AzSpeech::Internal::ProcessAzSpeechResult(SpeechSynthesisResult->Reason))
			{
				return *SpeechSynthesisResult->GetAudioData().get();
			}

			return std::vector<uint8_t>();
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncSSMLToStream(const FString& InSSML, const FSSMLToStreamDelegate& InDelegate)
		{
			if (InSSML.IsEmpty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: SSML is empty"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, InSSML, InDelegate]
			{
				const TFuture<std::vector<uint8_t>> SSMLToStreamAsyncWork = Async(EAsyncExecution::Thread, [=]() -> std::vector<uint8_t>
				{
					const std::string InSSMLStr = TCHAR_TO_UTF8(*InSSML);

					return Standard_Cpp::DoSSMLToStreamWork(InSSMLStr);
				});

				if (!SSMLToStreamAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
					return;
				}

				const std::vector<uint8_t> Result = SSMLToStreamAsyncWork.Get();
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

USSMLToStreamAsync* USSMLToStreamAsync::SSMLToStream(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToStreamAsync* const NewAsyncTask = NewObject<USSMLToStreamAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SSMLString = SSMLString;

	return NewAsyncTask;
}

void USSMLToStreamAsync::Activate()
{
	AzSpeechWrapper::Unreal_Cpp::AsyncSSMLToStream(SSMLString, TaskCompleted);
}
