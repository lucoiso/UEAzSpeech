// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToVoiceAsync.h"
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
		static bool DoTextToVoiceWork(const std::string& TextToConvert,
		                              const std::string& APIAccessKey,
		                              const std::string& RegionID,
		                              const std::string& LanguageID,
		                              const std::string& VoiceName)
		{
			const auto& SpeechConfig = SpeechConfig::FromSubscription(APIAccessKey, RegionID);

			SpeechConfig->SetSpeechSynthesisLanguage(LanguageID);
			SpeechConfig->SetSpeechSynthesisVoiceName(VoiceName);

			const auto& SpeechSynthesizer = SpeechSynthesizer::FromConfig(SpeechConfig);

			if (const auto& SpeechSynthesisResult = SpeechSynthesizer->SpeakTextAsync(TextToConvert).get();
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
		static void AsyncTextToVoice(const FString& TextToConvert,
		                             const FString& VoiceName,
		                             const FAzSpeechData Parameters,
		                             FTextToVoiceDelegate Delegate)
		{
			if (TextToConvert.IsEmpty() || VoiceName.IsEmpty()
				|| UAzSpeechHelper::IsAzSpeechDataEmpty(Parameters))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
			          [Parameters, TextToConvert, Delegate, VoiceName]
			          {
				          const TFuture<bool>& TextToVoiceAsyncWork =
					          Async(EAsyncExecution::Thread,
					                [Parameters, TextToConvert, VoiceName]() -> bool
					                {
						                const std::string& APIAccessKeyStr = TCHAR_TO_UTF8(*Parameters.APIAccessKey);
						                const std::string& RegionIDStr = TCHAR_TO_UTF8(*Parameters.RegionID);
						                const std::string& LanguageIDStr = TCHAR_TO_UTF8(*Parameters.LanguageID);
						                const std::string& VoiceNameStr = TCHAR_TO_UTF8(*VoiceName);
						                const std::string& ToConvertStr = TCHAR_TO_UTF8(*TextToConvert);

						                return Standard_Cpp::DoTextToVoiceWork(ToConvertStr,
						                                                       APIAccessKeyStr,
						                                                       RegionIDStr,
						                                                       LanguageIDStr,
						                                                       VoiceNameStr);
					                });

				          TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(5));
				          const bool& bOutputValue = TextToVoiceAsyncWork.Get();

				          AsyncTask(ENamedThreads::GameThread, [bOutputValue, Delegate]
				          {
					          Delegate.Broadcast(bOutputValue);
				          });

				          if (bOutputValue)
				          {
					          UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - AsyncTextToVoice: Result: Success"));
				          }
				          else
				          {
					          UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncTextToVoice: Result: Error"));
				          }
			          });
		}
	}
}

UTextToVoiceAsync* UTextToVoiceAsync::TextToVoiceAsync(const UObject* WorldContextObject,
                                                       const FString& TextToConvert,
                                                       const FString& VoiceName,
                                                       const FAzSpeechData Parameters)
{
	UTextToVoiceAsync* TextToVoiceAsync = NewObject<UTextToVoiceAsync>();
	TextToVoiceAsync->WorldContextObject = WorldContextObject;
	TextToVoiceAsync->TextToConvert = TextToConvert;
	TextToVoiceAsync->VoiceName = VoiceName;
	TextToVoiceAsync->Parameters = Parameters;

	return TextToVoiceAsync;
}

void UTextToVoiceAsync::Activate()
{
	AzSpeechWrapper::Unreal_Cpp::AsyncTextToVoice(TextToConvert, VoiceName, Parameters, TaskCompleted);
}
