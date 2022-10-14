// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/VoiceToTextAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeechInternalFuncs.h"

#if PLATFORM_ANDROID
// Only used to check android permission
#include "AzSpeech/AzSpeechHelper.h"
#endif

UVoiceToTextAsync* UVoiceToTextAsync::VoiceToText(const UObject* WorldContextObject, const FString& LanguageId)
{
	UVoiceToTextAsync* const VoiceToTextAsync = NewObject<UVoiceToTextAsync>();
	VoiceToTextAsync->WorldContextObject = WorldContextObject;
	VoiceToTextAsync->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);

	return VoiceToTextAsync;
}

void UVoiceToTextAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.RECORD_AUDIO");
#endif

	StartAzureTaskWork_Internal();
}

void UVoiceToTextAsync::StartAzureTaskWork_Internal()
{
	if (LanguageID.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
		return;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, this]
	{
		const TFuture<std::string> VoiceToTextAsyncWork = Async(EAsyncExecution::Thread, [=]() -> std::string
		{
			const std::string InLanguageIDStr = TCHAR_TO_UTF8(*LanguageID);

			return DoAzureTaskWork_Internal(InLanguageIDStr);
		});

		if (!VoiceToTextAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
			return;
		}

		const FString OutputValue = UTF8_TO_TCHAR(VoiceToTextAsyncWork.Get().c_str());
		AsyncTask(ENamedThreads::GameThread, [=]() { TaskCompleted.Broadcast(OutputValue); });

		if (!OutputValue.IsEmpty())
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: %s"), *FString(FuncName), *OutputValue);
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed"), *FString(FuncName));
		}
	});
}

std::string UVoiceToTextAsync::DoAzureTaskWork_Internal(const std::string& InLanguageID)
{
	const auto AudioConfig = AudioConfig::FromDefaultMicrophoneInput();
	const auto Recognizer = AzSpeech::Internal::GetAzureRecognizer(AudioConfig, InLanguageID);

	if (Recognizer == nullptr)
	{
		return std::string();
	}

	if (const auto RecognitionResult = Recognizer->RecognizeOnceAsync().get();
		AzSpeech::Internal::ProcessRecognitionResult(RecognitionResult))
	{
		return RecognitionResult->Text;
	}

	return std::string();
}
