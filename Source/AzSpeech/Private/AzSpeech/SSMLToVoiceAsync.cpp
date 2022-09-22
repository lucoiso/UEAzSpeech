// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToVoiceAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static bool DoSSMLToVoiceWork(const std::string& InSSML)
		{
			const auto Synthesizer = AzSpeech::Internal::GetAzureSynthesizer();

			if (Synthesizer == nullptr)
			{
				return false;
			}

			const auto SynthesisResult = Synthesizer->SpeakSsmlAsync(InSSML).get();

			return AzSpeech::Internal::ProcessAzSpeechResult(SynthesisResult->Reason);
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncSSMLToVoice(const FString& InSSML, const FSSMLToVoiceDelegate& InDelegate)
		{
			if (InSSML.IsEmpty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: SSML is empty"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));


			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, InSSML, &InDelegate]
			{
				const TFuture<bool> SSMLToVoiceAsyncWork = Async(EAsyncExecution::Thread, [=]() -> bool
				{
					const std::string InSSMLStr = TCHAR_TO_UTF8(*InSSML);

					return Standard_Cpp::DoSSMLToVoiceWork(InSSMLStr);
				});

				if (!SSMLToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
					return;
				}

				const bool bOutputValue = SSMLToVoiceAsyncWork.Get();
				AsyncTask(ENamedThreads::GameThread, [=]() { InDelegate.Broadcast(bOutputValue); });

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

USSMLToVoiceAsync* USSMLToVoiceAsync::SSMLToVoice(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToVoiceAsync* const SSMLToVoiceAsync = NewObject<USSMLToVoiceAsync>();
	SSMLToVoiceAsync->WorldContextObject = WorldContextObject;
	SSMLToVoiceAsync->SSMLString = SSMLString;

	return SSMLToVoiceAsync;
}

void USSMLToVoiceAsync::Activate()
{
	AzSpeechWrapper::Unreal_Cpp::AsyncSSMLToVoice(SSMLString, TaskCompleted);
}
