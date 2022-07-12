// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToStreamAsync.h"
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
		static std::vector<uint8_t> DoSSMLToStreamWork(const std::string& SSMLToConvert,
		                                               const std::string& APIAccessKey,
		                                               const std::string& RegionID)
		{
			const auto& SpeechConfig = SpeechConfig::FromSubscription(APIAccessKey, RegionID);

			const auto& SpeechSynthesizer =
				SpeechSynthesizer::FromConfig(SpeechConfig,
				                              AudioConfig::FromStreamOutput(AudioOutputStream::CreatePullStream()));

			if (const auto& SpeechSynthesisResult =
					SpeechSynthesizer->SpeakSsmlAsync(SSMLToConvert).get();
				SpeechSynthesisResult->Reason == ResultReason::SynthesizingAudioCompleted)
			{
				UE_LOG(LogAzSpeech, Display,
				       TEXT("AzSpeech - %s: Speech Synthesis task completed"), *FString(__func__));

				return *SpeechSynthesisResult->GetAudioData().get();
			}

			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Speech Synthesis task failed"), *FString(__func__));
			return std::vector<uint8_t>();
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncSSMLToStream(const FString& SSMLToConvert,
		                              const FAzSpeechData Parameters,
		                              FSSMLToStreamDelegate Delegate)
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
				          const TFuture<std::vector<uint8_t>>& TextToVoiceAsyncWork =
					          Async(EAsyncExecution::Thread,
					                [Parameters, SSMLToConvert, Delegate]() -> std::vector<uint8_t>
					                {
						                const std::string& APIAccessKeyStr = TCHAR_TO_UTF8(*Parameters.APIAccessKey);
						                const std::string& RegionIDStr = TCHAR_TO_UTF8(*Parameters.RegionID);
						                const std::string& ToConvertStr = TCHAR_TO_UTF8(*SSMLToConvert);

						                return Standard_Cpp::DoSSMLToStreamWork(
							                ToConvertStr, APIAccessKeyStr, RegionIDStr);
					                });

				          TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(5));

				          const std::vector<uint8_t>& Result = TextToVoiceAsyncWork.Get();
				          const bool& bOutputValue = !Result.empty();

				          TArray<uint8> OutputArr;

				          for (const uint8_t& i : Result)
				          {
					          OutputArr.Add(static_cast<uint8>(i));
				          }

				          AsyncTask(ENamedThreads::GameThread, [OutputArr, Delegate]
				          {
					          Delegate.Broadcast(OutputArr);
				          });

				          if (bOutputValue)
				          {
					          UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - AsyncTextToStream: Result: Success"));
				          }
				          else
				          {
					          UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncTextToStream: Result: Error"));
				          }
			          });
		}
	}
}

USSMLToStreamAsync* USSMLToStreamAsync::SSMLToStreamAsync(const UObject* WorldContextObject,
                                                          const FString& SSMLString,
                                                          const FAzSpeechData Parameters)
{
	USSMLToStreamAsync* NewAsyncTask = NewObject<USSMLToStreamAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SSMLString = SSMLString;
	NewAsyncTask->Parameters = Parameters;

	return NewAsyncTask;
}

void USSMLToStreamAsync::Activate()
{
	AzSpeechWrapper::Unreal_Cpp::AsyncSSMLToStream(SSMLString, Parameters, TaskCompleted);
}
