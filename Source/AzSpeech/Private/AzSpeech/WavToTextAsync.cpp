// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/WavToTextAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "HAL/PlatformFileManager.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static std::string DoWavToTextWork(const std::string& InFilePath, const std::string& InLanguageID)
		{
			const auto AudioConfig = AudioConfig::FromWavFileInput(InFilePath);
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
	}

	namespace Unreal_Cpp
	{
		static void AsyncWavToText(const FString& InFilePath, const FString& InFileName, const FString& InLanguageID, const FWavToTextDelegate& InDelegate)
		{
			if (InFilePath.IsEmpty() || InFileName.IsEmpty() || InLanguageID.IsEmpty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			const FString QualifiedPath = UAzSpeechHelper::QualifyWAVFileName(InFilePath, InFileName);
			if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*QualifiedPath))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: File not found"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, QualifiedPath, InLanguageID, InDelegate]
			{
				const TFuture<std::string> WavToTextAsyncWork = Async(EAsyncExecution::Thread, [=]() -> std::string
				{
					const std::string InFilePathStr = TCHAR_TO_UTF8(*QualifiedPath);
					const std::string InLanguageIDStr = TCHAR_TO_UTF8(*InLanguageID);

					return Standard_Cpp::DoWavToTextWork(InFilePathStr, InLanguageIDStr);
				});

				if (!WavToTextAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
					return;
				}

				const FString OutputValue = UTF8_TO_TCHAR(WavToTextAsyncWork.Get().c_str());
				AsyncTask(ENamedThreads::GameThread, [=] () { InDelegate.Broadcast(OutputValue); });

				if (!OutputValue.IsEmpty())
				{
					UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: %s"), *FString(FuncName), *OutputValue);
				}
				else
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Error"), *FString(FuncName));
				}
			});
		}
	}
}

UWavToTextAsync* UWavToTextAsync::WavToText(const UObject* WorldContextObject, const FString& FilePath, const FString& FileName, const FString& LanguageId)
{
	UWavToTextAsync* const WavToTextAsync = NewObject<UWavToTextAsync>();
	WavToTextAsync->WorldContextObject = WorldContextObject;
	WavToTextAsync->FilePath = FilePath;
	WavToTextAsync->FileName = FileName;
	WavToTextAsync->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);

	return WavToTextAsync;
}

void UWavToTextAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.READ_EXTERNAL_STORAGE");
#endif

	AzSpeechWrapper::Unreal_Cpp::AsyncWavToText(FilePath, FileName, LanguageID, TaskCompleted);
}
