// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToWavAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "HAL/PlatformFileManager.h"
#include "AzSpeech/AzSpeechHelper.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx.h>
THIRD_PARTY_INCLUDES_END

#if PLATFORM_ANDROID
#include "AndroidPermissionFunctionLibrary.h"
#endif

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

namespace AzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static bool DoTextToWavWork(const std::string& TextToConvert,
		                            const std::string& APIAccessKey,
		                            const std::string& RegionID,
		                            const std::string& LanguageID,
		                            const std::string& VoiceName,
		                            const std::string& FilePath)
		{
			const auto& SpeechConfig = SpeechConfig::FromSubscription(APIAccessKey, RegionID);

			SpeechConfig->SetSpeechSynthesisLanguage(LanguageID);
			SpeechConfig->SetSpeechSynthesisVoiceName(VoiceName);

			const auto& AudioConfig = AudioConfig::FromWavFileOutput(FilePath);
			const auto& SpeechSynthesizer = SpeechSynthesizer::FromConfig(SpeechConfig, AudioConfig);

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
		static void AsyncTextToWav(const FString& TextToConvert,
		                           const FString& VoiceName,
		                           const FString& FilePath,
		                           const FString& FileName,
		                           const FAzSpeechData Parameters,
		                           FTextToWavDelegate Delegate)
		{
			if (TextToConvert.IsEmpty() || VoiceName.IsEmpty()
				|| FilePath.IsEmpty() || FileName.IsEmpty()
				|| UAzSpeechHelper::IsAzSpeechDataEmpty(Parameters))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*FilePath))
			{
				UE_LOG(LogAzSpeech, Warning,
				       TEXT("AzSpeech - %s: Folder does not exist, trying to create a new with the specified path"),
				       *FString(__func__));

				if (FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*FilePath))
				{
					UE_LOG(LogAzSpeech, Display,
					       TEXT("AzSpeech - %s: Folder created with the specified path"),
					       *FString(__func__));
				}
				else
				{
					UE_LOG(LogAzSpeech, Error,
					       TEXT("AzSpeech - %s: Failed to create folder with the specified path"),
					       *FString(__func__));

					return;
				}
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
			          [Parameters, TextToConvert, Delegate, VoiceName, FilePath, FileName]
			          {
				          const TFuture<bool>& TextToVoiceAsyncWork =
					          Async(EAsyncExecution::Thread,
					                [Parameters, TextToConvert, VoiceName, FilePath, FileName]() -> bool
					                {
						                const std::string& APIAccessKeyStr = TCHAR_TO_UTF8(*Parameters.APIAccessKey);
						                const std::string& RegionIDStr = TCHAR_TO_UTF8(*Parameters.RegionID);
						                const std::string& LanguageIDStr = TCHAR_TO_UTF8(*Parameters.LanguageID);
						                const std::string& NameIDStr = TCHAR_TO_UTF8(*VoiceName);
						                const std::string& ToConvertStr = TCHAR_TO_UTF8(*TextToConvert);
						                const std::string& FilePathStr =
							                TCHAR_TO_UTF8(*UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName));

						                return Standard_Cpp::DoTextToWavWork(ToConvertStr, APIAccessKeyStr,
						                                                     RegionIDStr, LanguageIDStr, NameIDStr,
						                                                     FilePathStr);
					                });

				          TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(5));
				          const bool& bOutputValue = TextToVoiceAsyncWork.Get();

				          AsyncTask(ENamedThreads::GameThread, [bOutputValue, Delegate]
				          {
					          Delegate.Broadcast(bOutputValue);
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

UTextToWavAsync* UTextToWavAsync::TextToWavAsync(const UObject* WorldContextObject,
                                                 const FString& TextToConvert,
                                                 const FString& FilePath,
                                                 const FString& FileName,
                                                 const FString& VoiceName,
                                                 const FAzSpeechData Parameters)
{
	UTextToWavAsync* TextToWavAsync = NewObject<UTextToWavAsync>();
	TextToWavAsync->WorldContextObject = WorldContextObject;
	TextToWavAsync->TextToConvert = TextToConvert;
	TextToWavAsync->VoiceName = VoiceName;
	TextToWavAsync->FilePath = FilePath;
	TextToWavAsync->FileName = FileName;
	TextToWavAsync->Parameters = Parameters;

	return TextToWavAsync;
}

void UTextToWavAsync::Activate()
{
#if PLATFORM_ANDROID
	if (!UAndroidPermissionFunctionLibrary::CheckPermission(FString("android.permission.WRITE_EXTERNAL_STORAGE")))
	{
		UAndroidPermissionFunctionLibrary::AcquirePermissions(TArray<FString>{ ("android.permission.WRITE_EXTERNAL_STORAGE") });
	}
#endif

	AzSpeechWrapper::Unreal_Cpp::AsyncTextToWav(TextToConvert,
	                                            VoiceName,
	                                            FilePath,
	                                            FileName,
	                                            Parameters,
	                                            TaskCompleted);
}
