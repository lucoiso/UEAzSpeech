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
		static void AsyncSSMLToVoice(const FString& InSSML, FSSMLToVoiceDelegate Delegate)
		{
			if (InSSML.IsEmpty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: SSML is empty"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [InSSML, Delegate]
			{
				const TFuture<bool>& SSMLToVoiceAsyncWork = Async(EAsyncExecution::Thread, [InSSML]() -> bool
				{
					const std::string& InSSMLStr = TCHAR_TO_UTF8(*InSSML);

					return Standard_Cpp::DoSSMLToVoiceWork(InSSMLStr);
				});

				if (!SSMLToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(15)))
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncSSMLToVoice: Task timed out"));
					return;
				}
				
				const bool& bOutputValue = SSMLToVoiceAsyncWork.Get();

				AsyncTask(ENamedThreads::GameThread, [bOutputValue, Delegate]
				{
					Delegate.Broadcast(bOutputValue);
				});

				if (bOutputValue)
				{
					UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - AsyncSSMLToVoice: Result: Success"));
				}
				else
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncSSMLToVoice: Result: Error"));
				}
			});
		}
	}
}

USSMLToVoiceAsync* USSMLToVoiceAsync::SSMLToVoice(const UObject* WorldContextObject, const FString& SSMLString)
{
	USSMLToVoiceAsync* SSMLToVoiceAsync = NewObject<USSMLToVoiceAsync>();
	SSMLToVoiceAsync->WorldContextObject = WorldContextObject;
	SSMLToVoiceAsync->SSMLString = SSMLString;

	return SSMLToVoiceAsync;
}

void USSMLToVoiceAsync::Activate()
{
	AzSpeechWrapper::Unreal_Cpp::AsyncSSMLToVoice(SSMLString, TaskCompleted);
}
