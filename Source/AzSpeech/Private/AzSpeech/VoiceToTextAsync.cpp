// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/VoiceToTextAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

#if PLATFORM_ANDROID
#include "AzSpeech/AzSpeechHelper.h"
#endif

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static std::string DoVoiceToTextWork(const std::string& InLanguageID)
		{
			const auto AudioConfig = AudioConfig::FromDefaultMicrophoneInput();
			const auto Recognizer = AzSpeech::Internal::GetAzureRecognizer(AudioConfig, InLanguageID);

			if (Recognizer == nullptr)
			{
				return std::string();
			}

			if (const auto RecognitionResult = Recognizer->RecognizeOnceAsync().get();
				AzSpeech::Internal::ProcessAzSpeechResult(RecognitionResult->Reason))
			{
				return RecognitionResult->Text;
			}

			return std::string();
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncVoiceToText(const FString& InLanguageID, const FVoiceToTextDelegate& InDelegate)
		{
			if (InLanguageID.IsEmpty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [InLanguageID, InDelegate]
			{
				const TFuture<std::string> VoiceToTextAsyncWork =
					Async(EAsyncExecution::Thread, [InLanguageID]() -> std::string
					{
						const std::string InLanguageIDStr = TCHAR_TO_UTF8(*InLanguageID);

						return Standard_Cpp::DoVoiceToTextWork(InLanguageIDStr);
					});

				if (!VoiceToTextAsyncWork.WaitFor(FTimespan::FromSeconds(15)))
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncVoiceToText: Task timed out"));
					return;
				}
				
				const FString OutputValue = UTF8_TO_TCHAR(VoiceToTextAsyncWork.Get().c_str());

				AsyncTask(ENamedThreads::GameThread, [OutputValue, InDelegate]
				{
					InDelegate.Broadcast(OutputValue);
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

UVoiceToTextAsync* UVoiceToTextAsync::VoiceToText(const UObject* WorldContextObject,
                                                  const FString& LanguageId)
{
	UVoiceToTextAsync* VoiceToTextAsync = NewObject<UVoiceToTextAsync>();
	VoiceToTextAsync->WorldContextObject = WorldContextObject;
	VoiceToTextAsync->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);

	return VoiceToTextAsync;
}

void UVoiceToTextAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.RECORD_AUDIO");
#endif

	AzSpeechWrapper::Unreal_Cpp::AsyncVoiceToText(LanguageID, TaskCompleted);
}
