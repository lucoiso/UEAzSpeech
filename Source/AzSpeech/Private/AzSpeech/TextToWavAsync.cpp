// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToWavAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static bool DoTextToWavWork(const std::string& InStr,
		                            const std::string& InLanguageID,
		                            const std::string& InVoiceName,
		                            const std::string& InFilePath)
		{
			const auto AudioConfig = AudioConfig::FromWavFileOutput(InFilePath);
			const auto Synthesizer = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig, InLanguageID, InVoiceName);

			if (Synthesizer == nullptr)
			{
				return false;
			}

			const auto SynthesisResult = Synthesizer->SpeakTextAsync(InStr).get();

			return AzSpeech::Internal::ProcessAzSpeechResult(SynthesisResult->Reason);
		}
	}

	namespace Unreal_Cpp
	{
		static void AsyncTextToWav(const FString& InStr,
		                           const FString& InVoiceName,
		                           const FString& InFilePath,
		                           const FString& InFileName,
		                           const FString& InLanguageID,
		                           const FTextToWavDelegate& InDelegate)
		{
			if (InStr.IsEmpty() || InVoiceName.IsEmpty()
				|| InFilePath.IsEmpty() || InFileName.IsEmpty()
				|| InLanguageID.IsEmpty())
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

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
			          [InStr, InDelegate, InVoiceName, InFilePath, InFileName, InLanguageID]
			          {
				          const TFuture<bool> TextToWavAsyncWork =
							  Async(EAsyncExecution::Thread, [InStr, InVoiceName, InFilePath, InFileName, InLanguageID]() -> bool
							  {
								  const std::string InConvertStr = TCHAR_TO_UTF8(*InStr);
								  const std::string InLanguageIDStr = TCHAR_TO_UTF8(*InLanguageID);
								  const std::string InNameIDStr = TCHAR_TO_UTF8(*InVoiceName);
								  const std::string InFilePathStr = TCHAR_TO_UTF8(*UAzSpeechHelper::QualifyWAVFileName(InFilePath, 
																													   InFileName));

								  return Standard_Cpp::DoTextToWavWork(InConvertStr,
																	   InLanguageIDStr,
																	   InNameIDStr,
																	   InFilePathStr);
							  });

						  if (!TextToWavAsyncWork.WaitFor(FTimespan::FromSeconds(15)))
						  {
							  UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncTextToWav: Task timed out"));
							  return;
						  }
						
						  const bool bOutputValue = TextToWavAsyncWork.Get();

						  AsyncTask(ENamedThreads::GameThread, [bOutputValue, InDelegate]
						  {
							  InDelegate.Broadcast(bOutputValue);
						  });

						  if (bOutputValue)
						  {
							  UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - AsyncTextToWav: Result: Success"));
						  }
						  else
						  {
							  UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncTextToWav: Result: Error"));
						  }
			          });
		}
	}
}

UTextToWavAsync* UTextToWavAsync::TextToWav(const UObject* WorldContextObject,
                                            const FString& TextToConvert,
                                            const FString& FilePath,
                                            const FString& FileName,
                                            const FString& VoiceName,
                                            const FString& LanguageId)
{
	UTextToWavAsync* TextToWavAsync = NewObject<UTextToWavAsync>();
	TextToWavAsync->WorldContextObject = WorldContextObject;
	TextToWavAsync->TextToConvert = TextToConvert;
	TextToWavAsync->FilePath = FilePath;
	TextToWavAsync->FileName = FileName;
	TextToWavAsync->VoiceName = AzSpeech::Internal::GetVoiceName(VoiceName);
	TextToWavAsync->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);

	return TextToWavAsync;
}

void UTextToWavAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.WRITE_EXTERNAL_STORAGE");
#endif

	AzSpeechWrapper::Unreal_Cpp::AsyncTextToWav(TextToConvert,
	                                            VoiceName,
	                                            FilePath,
	                                            FileName,
	                                            LanguageID,
	                                            TaskCompleted);
}
