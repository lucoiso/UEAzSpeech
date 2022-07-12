// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToVoiceAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeech/AzSpeechHelper.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx.h>
THIRD_PARTY_INCLUDES_END

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static bool DoSSMLToVoiceWork(const std::string& SSMLToConvert,
		                              const std::string& APIAccessKey,
		                              const std::string& RegionID)
		{
			const auto& SpeechConfig = SpeechConfig::FromSubscription(APIAccessKey, RegionID);
			const auto& SpeechSynthesizer = SpeechSynthesizer::FromConfig(SpeechConfig);

			if (const auto& SpeechSynthesisResult = SpeechSynthesizer->SpeakSsmlAsync(SSMLToConvert).get();
				SpeechSynthesisResult->Reason == ResultReason::SynthesizingAudioCompleted)
			{
				UE_LOG(LogAzSpeech, Display,
				       TEXT("AzSpeech - %s: Speech Synthesis task completed"), *FString(__func__));

				return true;
			}

			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Speech Synthesis task failed"), *FString(__func__));
			return false;
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncSSMLToVoice(const FString& SSMLToConvert,
		                             const FAzSpeechData Parameters,
		                             FSSMLToVoiceDelegate Delegate)
		{
			if (SSMLToConvert.IsEmpty()
				|| UAzSpeechHelper::IsAzSpeechDataEmpty(Parameters))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
			          [Parameters, SSMLToConvert, Delegate]
			          {
				          const TFuture<bool>& SSMLToVoiceAsyncWork =
					          Async(EAsyncExecution::Thread,
					                [Parameters, SSMLToConvert]() -> bool
					                {
						                const std::string& APIAccessKeyStr = TCHAR_TO_UTF8(*Parameters.APIAccessKey);
						                const std::string& RegionIDStr = TCHAR_TO_UTF8(*Parameters.RegionID);
						                const std::string& SSMLStr = TCHAR_TO_UTF8(*SSMLToConvert);

						                return Standard_Cpp::DoSSMLToVoiceWork(SSMLStr, APIAccessKeyStr, RegionIDStr);
					                });

				          SSMLToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(5));
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

USSMLToVoiceAsync* USSMLToVoiceAsync::SSMLToVoiceAsync(const UObject* WorldContextObject,
                                                       const FString& SSMLString,
                                                       const FAzSpeechData Parameters)
{
	USSMLToVoiceAsync* SSMLToVoiceAsync = NewObject<USSMLToVoiceAsync>();
	SSMLToVoiceAsync->WorldContextObject = WorldContextObject;
	SSMLToVoiceAsync->SSMLString = SSMLString;
	SSMLToVoiceAsync->Parameters = Parameters;

	return SSMLToVoiceAsync;
}

void USSMLToVoiceAsync::Activate()
{
	AzSpeechWrapper::Unreal_Cpp::AsyncSSMLToVoice(SSMLString, Parameters, TaskCompleted);
}
