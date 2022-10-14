// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToWavAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static bool DoSSMLToWavWork(const std::string& InSSML, const std::string& InFilePath)
		{
			const auto AudioConfig = AudioConfig::FromWavFileOutput(InFilePath);
			const auto Synthesizer = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig);

			if (Synthesizer == nullptr)
			{
				return false;
			}

			const auto SynthesisResult = Synthesizer->SpeakSsmlAsync(InSSML).get();

			return AzSpeech::Internal::ProcessSynthesizResult(SynthesisResult);
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncSSMLToWav(const FString& InSSML, const FString& InFilePath, const FString& InFileName, const FSSMLToWavDelegate& InDelegate)
		{
			if (InSSML.IsEmpty() || InFilePath.IsEmpty() || InFileName.IsEmpty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			if (!UAzSpeechHelper::CreateNewDirectory(InFilePath))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Failed to create directory"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [FuncName = __func__, InSSML, InFilePath, InFileName, InDelegate]
			{
				const TFuture<bool> SSMLToWavAsyncWork = Async(EAsyncExecution::Thread, [=]() -> bool
				{
					const std::string InConvertStr = TCHAR_TO_UTF8(*InSSML);
					const std::string InFilePathStr = TCHAR_TO_UTF8(*UAzSpeechHelper::QualifyWAVFileName(InFilePath, InFileName));

					return Standard_Cpp::DoSSMLToWavWork(InConvertStr, InFilePathStr);
				});

				if (!SSMLToWavAsyncWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout())))
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task timed out"), *FString(FuncName));
					return;
				}

				const bool bOutputValue = SSMLToWavAsyncWork.Get();
				AsyncTask(ENamedThreads::GameThread, [=] () { InDelegate.Broadcast(bOutputValue); });

				if (bOutputValue)
				{
					UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(FuncName));
				}
				else
				{
					UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Error"), *FString(FuncName));
				}
			});
		}
	}
}

USSMLToWavAsync* USSMLToWavAsync::SSMLToWav(const UObject* WorldContextObject, const FString& SSMLString, const FString& FilePath, const FString& FileName)
{
	USSMLToWavAsync* const TextToWavAsync = NewObject<USSMLToWavAsync>();
	TextToWavAsync->WorldContextObject = WorldContextObject;
	TextToWavAsync->SSMLString = SSMLString;
	TextToWavAsync->FilePath = FilePath;
	TextToWavAsync->FileName = FileName;

	return TextToWavAsync;
}

void USSMLToWavAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.WRITE_EXTERNAL_STORAGE");
#endif

	AzSpeechWrapper::Unreal_Cpp::AsyncSSMLToWav(SSMLString, FilePath, FileName, TaskCompleted);
}
