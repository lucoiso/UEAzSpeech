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

UVoiceToTextAsync* UVoiceToTextAsync::VoiceToText(const UObject* WorldContextObject, const FString& LanguageId, const bool bContinuosRecognition)
{
	UVoiceToTextAsync* const VoiceToTextAsync = NewObject<UVoiceToTextAsync>();
	VoiceToTextAsync->WorldContextObject = WorldContextObject;
	VoiceToTextAsync->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);
	VoiceToTextAsync->bContinuousRecognition = bContinuosRecognition;

	return VoiceToTextAsync;
}

void UVoiceToTextAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.RECORD_AUDIO");
#endif

	Super::Activate();
}

bool UVoiceToTextAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (LanguageID.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
		return false;
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

		if (!OutputValue.Equals("CONTINUOUS_RECOGNITION"))
		{
			AsyncTask(ENamedThreads::GameThread, [=]() { if (CanBroadcast()) { TaskCompleted.Broadcast(OutputValue); } });
		}

		if (!OutputValue.IsEmpty())
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: %s"), *FString(FuncName), *OutputValue);
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed"), *FString(FuncName));
		}
	});

	return true;
}

std::string UVoiceToTextAsync::DoAzureTaskWork_Internal(const std::string& InLanguageID)
{
	const auto AudioConfig = AudioConfig::FromDefaultMicrophoneInput();
	RecognizerObject = AzSpeech::Internal::GetAzureRecognizer(AudioConfig, InLanguageID);

	if (RecognizerObject == nullptr)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Failed to proceed with task: RecognizerObject is null"), *FString(__func__));
		return std::string();
	}

	if (!bContinuousRecognition)
	{
		if (const auto RecognitionResult = RecognizerObject->RecognizeOnceAsync().get();
			AzSpeech::Internal::ProcessRecognitionResult(RecognitionResult))
		{
			return RecognitionResult->Text;
		}
	}
	else
	{			
		return StartContinuousRecognition();
	}

	return std::string();
}
