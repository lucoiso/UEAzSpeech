// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/VoiceToTextAsync.h"
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
		static std::string DoVoiceToTextWork(const std::string& APIAccessKey,
		                                     const std::string& RegionID,
		                                     const std::string& LanguageID)
		{
			const auto& Config = SpeechConfig::FromSubscription(APIAccessKey, RegionID);

			Config->SetSpeechRecognitionLanguage(LanguageID);
			Config->SetSpeechSynthesisLanguage(LanguageID);
			Config->SetProfanity(ProfanityOption::Raw);

			const auto& SpeechRecognizer = SpeechRecognizer::FromConfig(Config);

			if (const auto& SpeechRecognitionResult = SpeechRecognizer->RecognizeOnceAsync().get();
				SpeechRecognitionResult->Reason == ResultReason::RecognizedSpeech)
			{
				UE_LOG(LogAzSpeech, Display,
				       TEXT("AzSpeech - %s: Speech Recognition task completed"), *FString(__func__));

				return SpeechRecognitionResult->Text;
			}

			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Speech Recognition task failed"), *FString(__func__));
			return "";
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncVoiceToText(const FAzSpeechData Parameters,
		                             FVoiceToTextDelegate Delegate)
		{
			if (UAzSpeechHelper::IsAzSpeechDataEmpty(Parameters))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Parameters, Delegate]
			{
				const TFuture<std::string> VoiceToTextAsyncWork =
					Async(EAsyncExecution::Thread, [Parameters]() -> std::string
					{
						const std::string& APIAccessKey = TCHAR_TO_UTF8(*Parameters.APIAccessKey);
						const std::string& RegionID = TCHAR_TO_UTF8(*Parameters.RegionID);
						const std::string& LanguageID = TCHAR_TO_UTF8(*Parameters.LanguageID);

						return Standard_Cpp::DoVoiceToTextWork(APIAccessKey, RegionID, LanguageID);
					});

				VoiceToTextAsyncWork.WaitFor(FTimespan::FromSeconds(5));
				const FString& OutputValue = UTF8_TO_TCHAR(VoiceToTextAsyncWork.Get().c_str());

				AsyncTask(ENamedThreads::GameThread, [OutputValue, Delegate]
				{
					Delegate.Broadcast(OutputValue);
				});

				if (!OutputValue.IsEmpty())
				{
					UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - AsyncVoiceToText: Result: %s"), *OutputValue);
				}
				else
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncVoiceToText: Result: Error"));
				}
			});
		}
	}
}

UVoiceToTextAsync* UVoiceToTextAsync::VoiceToTextAsync(const UObject* WorldContextObject,
                                                       const FAzSpeechData Parameters)
{
	UVoiceToTextAsync* VoiceToTextAsync = NewObject<UVoiceToTextAsync>();
	VoiceToTextAsync->WorldContextObject = WorldContextObject;
	VoiceToTextAsync->Parameters = Parameters;
	return VoiceToTextAsync;
}

void UVoiceToTextAsync::Activate()
{
	AzSpeechWrapper::Unreal_Cpp::AsyncVoiceToText(Parameters, TaskCompleted);
}
