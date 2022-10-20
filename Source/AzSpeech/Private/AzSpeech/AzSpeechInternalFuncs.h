// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once
#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechSettings.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx.h>
THIRD_PARTY_INCLUDES_END

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

namespace AzSpeech::Internal
{
	static std::map<int, std::string> GetAzSpeechKeys()
	{
		std::map<int, std::string> Output;
		const UAzSpeechSettings* const Settings = GetDefault<UAzSpeechSettings>();

		const auto UpdateSettingsMap = [&Output](const int& InId, const FString& InString)
		{
			const std::string InStr = TCHAR_TO_UTF8(*InString);
			Output.insert(std::make_pair(InId, InStr));
		};

		UpdateSettingsMap(0, Settings->APIAccessKey);
		UpdateSettingsMap(1, Settings->RegionID);
		UpdateSettingsMap(2, Settings->LanguageID);
		UpdateSettingsMap(3, Settings->VoiceName);

		return Output;
	}

	static std::vector<std::string> GetCandidateLanguages()
	{
		std::vector<std::string> Output;

		const UAzSpeechSettings* const Settings = GetDefault<UAzSpeechSettings>();
		for (const FString& Iterator : Settings->AutoLanguageCandidates)
		{
			if (Iterator.IsEmpty())
			{
				continue;
			}

			Output.push_back(TCHAR_TO_UTF8(*Iterator));
		}

		return Output;
	}

	static float GetTimeout()
	{
		if (const UAzSpeechSettings* const Settings = GetDefault<UAzSpeechSettings>())
		{
			return Settings->TimeOutInSeconds;
		}

		return 15.f;
	}

	static FString GetLanguageID(const FString& InTestId = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		if (InTestId.IsEmpty() || InTestId.Equals("Default", ESearchCase::IgnoreCase))
		{
			return UTF8_TO_TCHAR(Settings.at(2).c_str());
		}

		return InTestId;
	}

	static FString GetVoiceName(const FString& InTestId = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		if (InTestId.IsEmpty() || InTestId.Equals("Default", ESearchCase::IgnoreCase))
		{
			return UTF8_TO_TCHAR(Settings.at(3).c_str());
		}

		return InTestId;
	}

	static FString GetAzSpeechLogsBaseDir()
	{
		return FPaths::ProjectSavedDir() + "Logs/UEAzSpeech";
	}

	static void EnableLogInConfiguration(const std::shared_ptr<SpeechConfig>& InConfig)
	{
		if (FString AzSpeechLogPath = GetAzSpeechLogsBaseDir();
			FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*AzSpeechLogPath))
		{
			AzSpeechLogPath += "/UEAzSpeech " + FDateTime::Now().ToString() + ".log";

			if (FFileHelper::SaveStringToFile(FString(), *AzSpeechLogPath))
			{
				InConfig->SetProperty(PropertyId::Speech_LogFilename, TCHAR_TO_UTF8(*AzSpeechLogPath));
			}
		}
	}

	static std::shared_ptr<SpeechSynthesizer> GetAzureSynthesizer(const std::shared_ptr<AudioConfig>& InAudioConfig = AudioConfig::FromDefaultSpeakerOutput(), const std::string& InLanguage = "Default", const std::string& InVoiceName = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		const auto SpeechConfig = SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));
				
		EnableLogInConfiguration(SpeechConfig);
		
		if (FString(UTF8_TO_TCHAR(InLanguage.c_str())).Equals("Auto", ESearchCase::IgnoreCase))
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing language auto detection..."), *FString(__func__));
			UE_LOG(LogAzSpeech, Warning, TEXT("AzSpeech - %s: Note: Synthesizers currently only support language detection from open range"), *FString(__func__));

			SpeechConfig->SetProperty(PropertyId::SpeechServiceConnection_SingleLanguageIdPriority, "Latency");
			return SpeechSynthesizer::FromConfig(SpeechConfig, AutoDetectSourceLanguageConfig::FromOpenRange(), InAudioConfig);
		}

		std::string UsedLang = InLanguage;
		if (FString LangStr_UE = UTF8_TO_TCHAR(InLanguage.c_str());
			LangStr_UE.Equals("Default", ESearchCase::IgnoreCase)
			|| LangStr_UE.IsEmpty())
		{
			UsedLang = TCHAR_TO_UTF8(*GetLanguageID(LangStr_UE));
		}

		std::string UsedVoice = InVoiceName;
		if (FString VoiceStr_UE = UTF8_TO_TCHAR(InVoiceName.c_str());
			VoiceStr_UE.Equals("Default", ESearchCase::IgnoreCase)
			|| VoiceStr_UE.IsEmpty())
		{
			UsedVoice = TCHAR_TO_UTF8(*GetVoiceName(VoiceStr_UE));
		}

		if (UsedVoice.empty() || UsedVoice.empty())
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task failed. Result: Invalid language or voice name"), *FString(__func__));
			return nullptr;
		}

		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Using language: %s"), *FString(__func__), *FString(UTF8_TO_TCHAR(InLanguage.c_str())));
		SpeechConfig->SetSpeechSynthesisLanguage(InLanguage);

		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Using voice: %s"), *FString(__func__), *FString(UTF8_TO_TCHAR(UsedVoice.c_str())));
		SpeechConfig->SetSpeechSynthesisVoiceName(UsedVoice);

		return SpeechSynthesizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	static std::shared_ptr<SpeechRecognizer> GetAzureRecognizer(const std::shared_ptr<AudioConfig>& InAudioConfig = AudioConfig::FromDefaultMicrophoneInput(), const std::string& InLanguage = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		const auto SpeechConfig = SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));

		EnableLogInConfiguration(SpeechConfig);

		SpeechConfig->SetProfanity(ProfanityOption::Raw);

		if (FString(UTF8_TO_TCHAR(InLanguage.c_str())).Equals("Auto", ESearchCase::IgnoreCase))
		{
			SpeechConfig->SetProperty(PropertyId::SpeechServiceConnection_SingleLanguageIdPriority, "Latency");

			const std::vector<std::string> Candidates = GetCandidateLanguages();
			if (Candidates.empty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task failed. Result: Invalid candidate languages"), *FString(__func__));

				return nullptr;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Initializing language auto detection"), *FString(__func__));
			for (const std::string& Iterator : Candidates)
			{
				UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Candidate: %s"), *FString(__func__), *FString(UTF8_TO_TCHAR(Iterator.c_str())));
			}

			return SpeechRecognizer::FromConfig(SpeechConfig, AutoDetectSourceLanguageConfig::FromLanguages(Candidates), InAudioConfig);
		}

		std::string UsedLang = InLanguage;
		if (FString LangStr_UE = UTF8_TO_TCHAR(InLanguage.c_str());
			LangStr_UE.Equals("Default", ESearchCase::IgnoreCase)
			|| LangStr_UE.IsEmpty())
		{
			UsedLang = TCHAR_TO_UTF8(*GetLanguageID(LangStr_UE));
		}

		if (UsedLang.empty())
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task failed. Result: Invalid language"), *FString(__func__));
			return nullptr;
		}

		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Using language: %s"), *FString(__func__), *FString(UTF8_TO_TCHAR(UsedLang.c_str())));

		SpeechConfig->SetSpeechRecognitionLanguage(UsedLang);
		SpeechConfig->SetSpeechSynthesisLanguage(UsedLang);

		return SpeechRecognizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	static FString CancellationReasonToString(const CancellationReason& CancellationReason)
	{
		switch (CancellationReason)
		{
			case CancellationReason::Error:
				return FString("Error");

			case CancellationReason::EndOfStream:
				return FString("EndOfStream");

			case CancellationReason::CancelledByUser:
				return FString("CancelledByUser");

			default:
				return FString("Undef");
		}
	}

	static void ProcessCancellationError(const CancellationErrorCode& ErrorCode, const std::string& ErrorDetails)
	{
		FString ErrorCodeStr;
		switch (ErrorCode)
		{
			case CancellationErrorCode::NoError:
				ErrorCodeStr = "Error";
				break;

			case CancellationErrorCode::AuthenticationFailure:
				ErrorCodeStr = "EndOfStream";
				break;

			case CancellationErrorCode::BadRequest:
				ErrorCodeStr = "CancelledByUser";
				break;

			case CancellationErrorCode::TooManyRequests:
				ErrorCodeStr = "CancelledByUser";
				break;

			case CancellationErrorCode::Forbidden:
				ErrorCodeStr = "CancelledByUser";
				break;

			case CancellationErrorCode::ConnectionFailure:
				ErrorCodeStr = "CancelledByUser";
				break;

			case CancellationErrorCode::ServiceTimeout:
				ErrorCodeStr = "CancelledByUser";
				break;

			case CancellationErrorCode::ServiceError:
				ErrorCodeStr = "CancelledByUser";
				break;

			case CancellationErrorCode::ServiceUnavailable:
				ErrorCodeStr = "CancelledByUser";
				break;

			case CancellationErrorCode::RuntimeError:
				ErrorCodeStr = "CancelledByUser";
				break;

			case CancellationErrorCode::ServiceRedirectTemporary:
				ErrorCodeStr = "CancelledByUser";
				break;

			case CancellationErrorCode::ServiceRedirectPermanent:
				ErrorCodeStr = "CancelledByUser";
				break;

			case CancellationErrorCode::EmbeddedModelError:
				ErrorCodeStr = "CancelledByUser";
				break;

			default:
				ErrorCodeStr = "Undef";
				break;
		}

		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Error code: %s"), *FString(__func__), *ErrorCodeStr);

		const FString ErrorDetailsStr = UTF8_TO_TCHAR(ErrorDetails.c_str());
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Error Details: %s"), *FString(__func__), *ErrorDetailsStr);		
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Log generated in directory: %s"), *FString(__func__), *GetAzSpeechLogsBaseDir());
	}

	static bool ProcessRecognitionResult(const std::shared_ptr<SpeechRecognitionResult>& Result)
	{
		if (Result->Reason == ResultReason::RecognizedSpeech)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Task completed. Reason: RecognizedSpeech"), *FString(__func__));
			return true;
		}
		else if (Result->Reason == ResultReason::Canceled)
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task failed. Reason: Canceled"), *FString(__func__));
			const auto CancellationDetails = CancellationDetails::FromResult(Result);

			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Cancellation Reason: %s"), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

			if (CancellationDetails->Reason == CancellationReason::Error)
			{
				ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
			}

			return false;
		}

		UE_LOG(LogAzSpeech, Warning, TEXT("AzSpeech - %s: Undefined reason"), *FString(__func__));
		return false;
	}

	static bool ProcessSynthesizResult(const std::shared_ptr<SpeechSynthesisResult>& Result)
	{
		if (Result->Reason == ResultReason::SynthesizingAudioCompleted)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Task completed. Reason: SynthesizingAudioCompleted"), *FString(__func__));
			return true;
		}
		else if (Result->Reason == ResultReason::Canceled)
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Task failed. Reason: Canceled"), *FString(__func__));
			const auto CancellationDetails = SpeechSynthesisCancellationDetails::FromResult(Result);

			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Cancellation Reason: %s"), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

			if (CancellationDetails->Reason == CancellationReason::Error)
			{
				ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
			}

			return false;
		}

		UE_LOG(LogAzSpeech, Warning, TEXT("AzSpeech - %s: Undefined reason"), *FString(__func__));
		return false;
	}
}
