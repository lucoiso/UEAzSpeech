// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/WavToTextAsync.h"
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
		static std::string DoWavToTextWork(const std::string& FilePath,
		                                   const std::string& APIAccessKey,
		                                   const std::string& RegionID,
		                                   const std::string& LanguageID)
		{
			const auto& SpeechConfig = SpeechConfig::FromSubscription(APIAccessKey, RegionID);

			SpeechConfig->SetSpeechRecognitionLanguage(LanguageID);
			SpeechConfig->SetSpeechSynthesisLanguage(LanguageID);
			SpeechConfig->SetProfanity(ProfanityOption::Raw);

			const auto& AudioConfig = AudioConfig::FromWavFileInput(FilePath);
			const auto& SpeechRecognizer = SpeechRecognizer::FromConfig(SpeechConfig, AudioConfig);

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
		static void AsyncWavToText(const FString& FilePath,
		                           const FString& FileName,
		                           const FAzSpeechData Parameters,
		                           FWavToTextDelegate Delegate)
		{
			if (FilePath.IsEmpty() || FileName.IsEmpty()
				|| UAzSpeechHelper::IsAzSpeechDataEmpty(Parameters))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Missing parameters"), *FString(__func__));
				return;
			}

			const FString& QualifiedPath = UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName);

			if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*QualifiedPath))
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: File not found"), *FString(__func__));
				return;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing task"), *FString(__func__));

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
			          [Parameters, Delegate, QualifiedPath]
			          {
				          const TFuture<std::string>& WavToTextAsyncWork =
					          Async(EAsyncExecution::Thread,
					                [Parameters, QualifiedPath]() -> std::string
					                {
						                const std::string& APIAccessKeyStr = TCHAR_TO_UTF8(*Parameters.APIAccessKey);
						                const std::string& RegionIDStr = TCHAR_TO_UTF8(*Parameters.RegionID);
						                const std::string& LanguageIDStr = TCHAR_TO_UTF8(*Parameters.LanguageID);
						                const std::string& FilePathStr = TCHAR_TO_UTF8(*QualifiedPath);

						                return Standard_Cpp::DoWavToTextWork(
							                FilePathStr, APIAccessKeyStr, RegionIDStr, LanguageIDStr);
					                });

				          WavToTextAsyncWork.WaitFor(FTimespan::FromSeconds(5));
				          const FString& OutputValue = UTF8_TO_TCHAR(WavToTextAsyncWork.Get().c_str());

				          AsyncTask(ENamedThreads::GameThread, [OutputValue, Delegate]
				          {
					          Delegate.Broadcast(OutputValue);
				          });

				          if (!OutputValue.IsEmpty())
				          {
					          UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - AsyncWavToText: Result: %s"), *OutputValue);
				          }
				          else
				          {
					          UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - AsyncWavToText: Result: Error"));
				          }
			          });
		}
	}
}

UWavToTextAsync* UWavToTextAsync::WavToTextAsync(const UObject* WorldContextObject,
                                                 const FString& FilePath,
                                                 const FString& FileName,
                                                 const FAzSpeechData Parameters)
{
	UWavToTextAsync* WavToTextAsync = NewObject<UWavToTextAsync>();
	WavToTextAsync->WorldContextObject = WorldContextObject;
	WavToTextAsync->Parameters = Parameters;
	WavToTextAsync->FilePath = FilePath;
	WavToTextAsync->FileName = FileName;

	return WavToTextAsync;
}

void UWavToTextAsync::Activate()
{
#if PLATFORM_ANDROID
	if (!UAndroidPermissionFunctionLibrary::CheckPermission(FString("android.permission.READ_EXTERNAL_STORAGE")))
	{
		UAndroidPermissionFunctionLibrary::AcquirePermissions(TArray<FString>{ ("android.permission.READ_EXTERNAL_STORAGE") });
	}
#endif

	AzSpeechWrapper::Unreal_Cpp::AsyncWavToText(FilePath, FileName, Parameters, TaskCompleted);
}
