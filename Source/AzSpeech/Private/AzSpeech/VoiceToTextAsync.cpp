// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/VoiceToTextAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static std::string DoVoiceToTextWork(const std::string& InLanguageID)
		{
			const auto& SpeechRecognizer = AzSpeech::Internal::GetAzureRecognizer(nullptr, InLanguageID);

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
		static void AsyncVoiceToText(const FString& InLanguageID, FVoiceToTextDelegate InDelegate)
		{
			if (InLanguageID.IsEmpty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [InLanguageID, InDelegate]
			{
				const TFuture<std::string>& VoiceToTextAsyncWork =
					Async(EAsyncExecution::Thread, [InLanguageID]() -> std::string
					{
						const std::string& InLanguageIDStr = TCHAR_TO_UTF8(*InLanguageID);

						return Standard_Cpp::DoVoiceToTextWork(InLanguageIDStr);
					});

				VoiceToTextAsyncWork.WaitFor(FTimespan::FromSeconds(5));
				const FString& OutputValue = UTF8_TO_TCHAR(VoiceToTextAsyncWork.Get().c_str());

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

UVoiceToTextAsync* UVoiceToTextAsync::VoiceToText(const UObject* WorldContextObject, const FString& LanguageId)
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
