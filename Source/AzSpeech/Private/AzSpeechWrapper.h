// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "AzSpeechData.h"
#include "Async/Async.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx.h>
THIRD_PARTY_INCLUDES_END

/**
 *
 */
	namespace FAzSpeechWrapper
{
	namespace Standard_Cpp
	{
		static std::string DoVoiceToTextWork(const std::string APIAccessKey, const std::string RegionID,
			const std::string LanguageID)
		{
			if (APIAccessKey.size() == 0 || RegionID.size() == 0 || LanguageID.size() == 0)
			{
				return "Invalid parameters";
			}

			const auto& Config = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(
				APIAccessKey, RegionID);

			Config->SetSpeechRecognitionLanguage(LanguageID);
			Config->SetSpeechSynthesisLanguage(LanguageID);
			Config->SetProfanity(Microsoft::CognitiveServices::Speech::ProfanityOption::Raw);

			const auto& SpeechRecognizer = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(Config);
			const auto& SpeechRecognitionResult = SpeechRecognizer->RecognizeOnceAsync().get();

			std::string RecognizedString = "not recognized";

			if (SpeechRecognitionResult->Reason == Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech)
			{
				RecognizedString = SpeechRecognitionResult->Text;
			}

			return RecognizedString;
		}

		static bool DoTextToVoiceWork(const std::string TextToConvert, const std::string APIAccessKey,
			const std::string RegionID, const std::string LanguageID, const std::string VoiceName)
		{
			if (APIAccessKey.size() == 0 || RegionID.size() == 0 ||
				LanguageID.size() == 0 || TextToConvert.size() == 0)
			{
				return false;
			}

			const auto& SpeechConfig = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(
				APIAccessKey, RegionID);

			SpeechConfig->SetSpeechSynthesisLanguage(LanguageID);
			SpeechConfig->SetSpeechSynthesisVoiceName(VoiceName);

			const auto& SpeechSynthesizer = Microsoft::CognitiveServices::Speech::SpeechSynthesizer::FromConfig(
				SpeechConfig);

			const auto& SpeechSynthesisResult = SpeechSynthesizer->SpeakTextAsync(TextToConvert).get();

			if (SpeechSynthesisResult->Reason ==
				Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioCompleted)
			{
				return true;
			}

			return false;
		}

		static bool DoTextToWavWork(const std::string TextToConvert, const std::string APIAccessKey,
			const std::string RegionID, const std::string LanguageID,
			const std::string VoiceName, const std::string FilePath, const std::string FileName)
		{
			if (APIAccessKey.size() == 0 || RegionID.size() == 0 ||
				LanguageID.size() == 0 || TextToConvert.size() == 0 ||
				FilePath.size() == 0 || FileName.size() == 0)
			{
				return false;
			}

			const auto& SpeechConfig = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(APIAccessKey, RegionID);

			SpeechConfig->SetSpeechSynthesisLanguage(LanguageID);
			SpeechConfig->SetSpeechSynthesisVoiceName(VoiceName);

			const auto QualifiedFileInfo = [FilePath, FileName]() -> const std::string
			{
				std::string LocalPath = FilePath;
				if (*FilePath.end() != '\\')
				{
					LocalPath += '\\';
				}

				std::string LocalName = FileName;
				if (FileName.substr(FileName.size() - 4, 4) != ".wav")
				{
					LocalName += ".wav";
				}

				return LocalPath + LocalName;
			};

			const auto& AudioConfig = Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromWavFileOutput(QualifiedFileInfo());
			const auto& SpeechSynthesizer = Microsoft::CognitiveServices::Speech::SpeechSynthesizer::FromConfig(SpeechConfig, AudioConfig);
			const auto& SpeechSynthesisResult = SpeechSynthesizer->SpeakTextAsync(TextToConvert).get();

			if (SpeechSynthesisResult->Reason ==
				Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioCompleted)
			{
				return true;
			}

			return false;
		}

		static std::string DoWavToTextWork(const std::string FilePath, const std::string FileName,
			const std::string APIAccessKey, const std::string RegionID, const std::string LanguageID)
		{
			if (APIAccessKey.size() == 0 || RegionID.size() == 0 ||
				LanguageID.size() == 0 || FilePath.size() == 0 || FileName.size() == 0)
			{
				return "Invalid parameters";
			}

			const auto& SpeechConfig = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(APIAccessKey, RegionID);

			SpeechConfig->SetSpeechRecognitionLanguage(LanguageID);
			SpeechConfig->SetSpeechSynthesisLanguage(LanguageID);
			SpeechConfig->SetProfanity(Microsoft::CognitiveServices::Speech::ProfanityOption::Raw);

			const auto QualifiedFileInfo = [FilePath, FileName]() -> const std::string
			{
				std::string LocalPath = FilePath;
				if (*FilePath.end() != '\\')
				{
					LocalPath += '\\';
				}

				std::string LocalName = FileName;
				if (FileName.substr(FileName.size() - 4, 4) != ".wav")
				{
					LocalName += ".wav";
				}

				return LocalPath + LocalName;
			};

			const auto& AudioConfig = Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromWavFileInput(QualifiedFileInfo());
			const auto& SpeechRecognizer = Microsoft::CognitiveServices::Speech::SpeechRecognizer::FromConfig(SpeechConfig, AudioConfig);
			const auto& SpeechRecognitionResult = SpeechRecognizer->RecognizeOnceAsync().get();

			std::string RecognizedString = "not recognized";

			if (SpeechRecognitionResult->Reason == Microsoft::CognitiveServices::Speech::ResultReason::RecognizedSpeech)
			{
				RecognizedString = SpeechRecognitionResult->Text;
			}

			return RecognizedString;
		}

		static std::vector<uint8_t> DoTextToStreamWork(const std::string TextToConvert, const std::string APIAccessKey,
			const std::string RegionID, const std::string LanguageID, const std::string VoiceName)
		{
			if (APIAccessKey.size() == 0 || RegionID.size() == 0 ||
				LanguageID.size() == 0 || TextToConvert.size() == 0)
			{
				return std::vector<uint8_t>();
			}

			const auto& SpeechConfig = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(APIAccessKey, RegionID);

			SpeechConfig->SetSpeechSynthesisLanguage(LanguageID);
			SpeechConfig->SetSpeechSynthesisVoiceName(VoiceName);

			const auto& SpeechSynthesizer = Microsoft::CognitiveServices::Speech::SpeechSynthesizer::FromConfig(SpeechConfig,
				Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromStreamOutput(
					Microsoft::CognitiveServices::Speech::Audio::AudioOutputStream::CreatePullStream()));

			const auto& SpeechSynthesisResult = SpeechSynthesizer->SpeakTextAsync(TextToConvert).get();

			if (SpeechSynthesisResult->Reason ==
				Microsoft::CognitiveServices::Speech::ResultReason::SynthesizingAudioCompleted)
			{
				return *SpeechSynthesisResult->GetAudioData().get();
			}

			return std::vector<uint8_t>();
		}
	} // namespace Standard_Cpp

	namespace Unreal_Cpp
	{
		static void AsyncVoiceToText(const FAzSpeechData Parameters, FVoiceToTextDelegate Delegate)
		{
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Parameters, Delegate]()
				{
					const TFuture<std::string> VoiceToTextAsyncWork =
						Async(EAsyncExecution::Thread, [Parameters]() -> std::string
							{
								const std::string APIAccessKey = TCHAR_TO_UTF8(*Parameters.APIAccessKey);
								const std::string RegionID = TCHAR_TO_UTF8(*Parameters.RegionID);
								const std::string LanguageID = TCHAR_TO_UTF8(*Parameters.LanguageID);

								return Standard_Cpp::DoVoiceToTextWork(APIAccessKey, RegionID, LanguageID);
							});

					VoiceToTextAsyncWork.WaitFor(FTimespan::FromSeconds(5));
					const FString& RecognizedString = UTF8_TO_TCHAR(VoiceToTextAsyncWork.Get().c_str());

					AsyncTask(ENamedThreads::GameThread, [RecognizedString, Delegate]()
						{
							Delegate.Broadcast(RecognizedString);
						});

					UE_LOG(LogTemp, Warning,
						TEXT("AzSpeech Debug - API Access Key: %s, Region: %s, Language: %s, Voice-To-Text Result: %s"),
						*FString(Parameters.APIAccessKey), *FString(Parameters.RegionID),
						*FString(Parameters.LanguageID),
						*RecognizedString);
				});
		}

		static void AsyncTextToVoice(const FString TextToConvert,
			const FString VoiceName, const FAzSpeechData Parameters, FTextToVoiceDelegate Delegate)
		{
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Parameters, TextToConvert, Delegate, VoiceName]()
				{
					const TFuture<bool> TextToVoiceAsyncWork =
						Async(EAsyncExecution::Thread, [Parameters, TextToConvert, VoiceName]() -> bool
							{
								const std::string APIAccessKeyStr = TCHAR_TO_UTF8(*Parameters.APIAccessKey);
								const std::string RegionIDStr = TCHAR_TO_UTF8(*Parameters.RegionID);
								const std::string LanguageIDStr = TCHAR_TO_UTF8(*Parameters.LanguageID);
								const std::string VoiceNameStr = TCHAR_TO_UTF8(*VoiceName);
								const std::string ToConvertStr = TCHAR_TO_UTF8(*TextToConvert);

								return Standard_Cpp::DoTextToVoiceWork(ToConvertStr, APIAccessKeyStr, RegionIDStr, LanguageIDStr,
									VoiceNameStr);
							});

					TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(5));
					const bool bOutputValue = TextToVoiceAsyncWork.Get();

					AsyncTask(ENamedThreads::GameThread, [bOutputValue, Delegate]()
						{
							Delegate.Broadcast(bOutputValue);
						});

					const FString OutputValueStr = bOutputValue ? "Success" : "Error";

					UE_LOG(LogTemp, Warning,
						TEXT("AzSpeech Debug - API Access Key: %s, Region: %s, Language: %s, Text-To-Voice Result: %s"),
						*FString(Parameters.APIAccessKey), *FString(Parameters.RegionID),
						*FString(Parameters.LanguageID),
						*OutputValueStr);
				});
		}

		static void AsyncTextToWav(const FString TextToConvert, const FString VoiceName, const FString FilePath, const FString FileName, const FAzSpeechData Parameters, FTextToWavDelegate Delegate)
		{
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Parameters, TextToConvert, Delegate, VoiceName, FilePath, FileName]()
				{
					const TFuture<bool> TextToVoiceAsyncWork =
						Async(EAsyncExecution::Thread, [Parameters, TextToConvert, VoiceName, FilePath, FileName]() -> bool
							{
								const std::string APIAccessKeyStr = TCHAR_TO_UTF8(*Parameters.APIAccessKey);
								const std::string RegionIDStr = TCHAR_TO_UTF8(*Parameters.RegionID);
								const std::string LanguageIDStr = TCHAR_TO_UTF8(*Parameters.LanguageID);
								const std::string NameIDStr = TCHAR_TO_UTF8(*VoiceName);
								const std::string FilePathStr = TCHAR_TO_UTF8(*FilePath);
								const std::string FileNameStr = TCHAR_TO_UTF8(*FileName);
								const std::string ToConvertStr = TCHAR_TO_UTF8(*TextToConvert);

								return Standard_Cpp::DoTextToWavWork(ToConvertStr, APIAccessKeyStr, RegionIDStr, LanguageIDStr,
									NameIDStr, FilePathStr, FileNameStr);
							});

					TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(5));
					const bool bOutputValue = TextToVoiceAsyncWork.Get();

					AsyncTask(ENamedThreads::GameThread, [bOutputValue, Delegate]()
						{
							Delegate.Broadcast(bOutputValue);
						});

					const FString OutputValueStr = bOutputValue ? "Success" : "Error";

					UE_LOG(LogTemp, Warning,
						TEXT("AzSpeech Debug - API Access Key: %s, Region: %s, Language: %s, Text-To-Wav Result: %s"),
						*FString(Parameters.APIAccessKey), *FString(Parameters.RegionID),
						*FString(Parameters.LanguageID),
						*OutputValueStr);
				});
		}

		static void AsyncWavToText(const FString FilePath, const FString FileName, const FAzSpeechData Parameters,
			FWavToTextDelegate Delegate)
		{
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Parameters, FilePath, FileName, Delegate]()
				{
					const TFuture<std::string> WavToTextAsyncWork =
						Async(EAsyncExecution::Thread, [Parameters, FilePath, FileName]() -> std::string
							{
								const std::string APIAccessKeyStr = TCHAR_TO_UTF8(*Parameters.APIAccessKey);
								const std::string RegionIDStr = TCHAR_TO_UTF8(*Parameters.RegionID);
								const std::string LanguageIDStr = TCHAR_TO_UTF8(*Parameters.LanguageID);
								const std::string FilePathStr = TCHAR_TO_UTF8(*FilePath);
								const std::string FileNameStr = TCHAR_TO_UTF8(*FileName);

								return Standard_Cpp::DoWavToTextWork(FilePathStr, FileNameStr, APIAccessKeyStr, RegionIDStr,
									LanguageIDStr);
							});

					WavToTextAsyncWork.WaitFor(FTimespan::FromSeconds(5));
					const FString bOutputValue = UTF8_TO_TCHAR(WavToTextAsyncWork.Get().c_str());

					AsyncTask(ENamedThreads::GameThread, [bOutputValue, Delegate]()
						{
							Delegate.Broadcast(bOutputValue);
						});

					UE_LOG(LogTemp, Warning,
						TEXT("AzSpeech Debug - API Access Key: %s, Region: %s, Language: %s, Wav-To-Text Result: %s"),
						*FString(Parameters.APIAccessKey), *FString(Parameters.RegionID),
						*FString(Parameters.LanguageID),
						*bOutputValue);
				});
		}

		static void AsyncTextToStream(const FString TextToConvert, const FString VoiceName,
			const FAzSpeechData Parameters, FTextToStreamDelegate Delegate)
		{
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Parameters, TextToConvert, Delegate, VoiceName]()
				{
					const TFuture<std::vector<uint8_t>> TextToVoiceAsyncWork =
						Async(EAsyncExecution::Thread, [Parameters, TextToConvert, VoiceName]() -> std::vector<uint8_t>
							{
								const std::string APIAccessKeyStr = TCHAR_TO_UTF8(*Parameters.APIAccessKey);
								const std::string RegionIDStr = TCHAR_TO_UTF8(*Parameters.RegionID);
								const std::string LanguageIDStr = TCHAR_TO_UTF8(*Parameters.LanguageID);
								const std::string NameIDStr = TCHAR_TO_UTF8(*VoiceName);
								const std::string ToConvertStr = TCHAR_TO_UTF8(*TextToConvert);

								return Standard_Cpp::DoTextToStreamWork(ToConvertStr, APIAccessKeyStr, RegionIDStr, LanguageIDStr, NameIDStr);
							});

					TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(5));

					std::vector<uint8_t> Result = TextToVoiceAsyncWork.Get();
					const bool bOutputValue = !Result.empty();

					TArray<uint8> OutputArr;

					for (const uint8_t i : Result)
					{
						OutputArr.Add(static_cast<uint8>(i));
					}

					AsyncTask(ENamedThreads::GameThread, [OutputArr, Delegate]()
						{
							Delegate.Broadcast(OutputArr);
						});

					const FString OutputValueStr = bOutputValue ? "Success" : "Error";

					UE_LOG(LogTemp, Warning,
						TEXT("AzSpeech Debug - API Access Key: %s, Region: %s, Language: %s, Text-To-Stream Result: %s"),
						*FString(Parameters.APIAccessKey), *FString(Parameters.RegionID),
						*FString(Parameters.LanguageID),
						*OutputValueStr);
				});
		}
	} // namespace Unreal_Cpp
} // namespace FAzSpeechWrapper
