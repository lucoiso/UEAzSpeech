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
	const UAzSpeechSettings* GetPluginSettings()
	{
		static const UAzSpeechSettings* const Instance = GetDefault<UAzSpeechSettings>();
		return Instance;
	}

	const std::map<int, std::string> GetAzSpeechKeys()
	{
		std::map<int, std::string> Output;
		const UAzSpeechSettings* const Settings = GetPluginSettings();
			
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

	const bool CheckAzSpeechSettings()
	{
		const auto AzSpeechParams = GetAzSpeechKeys();
		if (AzSpeechParams.empty())
		{
			return false;
		}

		for (uint8 Iterator = 0u; Iterator < AzSpeechParams.size(); ++Iterator)
		{
			if (AzSpeechParams.at(Iterator).empty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("%s: Invalid settings. Check your AzSpeech settings on Project Settings -> AzSpeech Settings."), *FString(__func__));
				return false;
			}
		}

		return true;
	}

	const std::vector<std::string> GetCandidateLanguages()
	{
		std::vector<std::string> Output;

		const UAzSpeechSettings* const Settings = GetPluginSettings();
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

	const int32 GetTimeout()
	{
		if (const UAzSpeechSettings* const Settings = GetPluginSettings())
		{
			return Settings->TimeOutInSeconds;
		}

		return 15.f;
	}

	const ProfanityOption GetProfanityFilter()
	{
		if (const UAzSpeechSettings* const Settings = GetPluginSettings())
		{
			switch (Settings->ProfanityFilter)
			{
				case EAzSpeechProfanityFilter::Raw :
					return ProfanityOption::Raw;

				case EAzSpeechProfanityFilter::Removed:
					return ProfanityOption::Removed;

				case EAzSpeechProfanityFilter::Masked:
					return ProfanityOption::Masked;
					
				default: break;
			}
		}

		return ProfanityOption::Raw;
	}

	const FString GetLanguageID(const FString& InTestId = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		if (InTestId.IsEmpty() || InTestId.Equals("Default", ESearchCase::IgnoreCase))
		{
			return UTF8_TO_TCHAR(Settings.at(2).c_str());
		}

		return InTestId;
	}

	const FString GetVoiceName(const FString& InTestId = "Default")
	{
		const auto Settings = GetAzSpeechKeys();
		if (InTestId.IsEmpty() || InTestId.Equals("Default", ESearchCase::IgnoreCase))
		{
			return UTF8_TO_TCHAR(Settings.at(3).c_str());
		}

		return InTestId;
	}

	const FString GetAzSpeechLogsBaseDir()
	{
		return FPaths::ProjectSavedDir() + "Logs/UEAzSpeech";
	}

	void EnableLogInConfiguration(const std::shared_ptr<SpeechConfig>& InConfig)
	{
		if (!GetPluginSettings()->bEnableSDKLogs)
		{
			return;
		}

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

	std::string GetValidatedLanguageStr(const std::string& InLanguage)
	{
		std::string UsedLang = InLanguage;
		if (const FString LangStr_UE = UTF8_TO_TCHAR(InLanguage.c_str());
			LangStr_UE.Equals("Default", ESearchCase::IgnoreCase) || LangStr_UE.IsEmpty())
		{
			UsedLang = TCHAR_TO_UTF8(*GetLanguageID(LangStr_UE));
		}

		return UsedLang;
	}

	std::string GetValidatedVoiceNameStr(const std::string& InVoiceName)
	{
		std::string UsedVoice = InVoiceName;
		if (const FString VoiceStr_UE = UTF8_TO_TCHAR(InVoiceName.c_str());
			VoiceStr_UE.Equals("Default", ESearchCase::IgnoreCase) || VoiceStr_UE.IsEmpty())
		{
			UsedVoice = TCHAR_TO_UTF8(*GetVoiceName(VoiceStr_UE));
		}

		return UsedVoice;
	}

	bool IsUsingAutoLanguage(const std::string& InLanguage)
	{
		return FString(UTF8_TO_TCHAR(InLanguage.c_str())).Equals("Auto", ESearchCase::IgnoreCase);
	}

	bool AddSharedSDKSettings(const std::shared_ptr<SpeechConfig>& InConfig, const std::string& InLanguage, bool& bIsUsingAutoLang)
	{
		EnableLogInConfiguration(InConfig);

		InConfig->SetProfanity(GetProfanityFilter());

		bIsUsingAutoLang = IsUsingAutoLanguage(InLanguage);

		if (bIsUsingAutoLang)
		{
			InConfig->SetProperty(PropertyId::SpeechServiceConnection_SingleLanguageIdPriority, "Latency");
		}

		return true;
	}

	bool AddRecognitionSDKSettings(const std::shared_ptr<SpeechConfig>& InConfig, const std::string& InLanguage, bool& bIsUsingAutoLang)
	{
		if (!AddSharedSDKSettings(InConfig, InLanguage, bIsUsingAutoLang))
		{
			return false;
		}

		if (bIsUsingAutoLang)
		{
			return true;
		}

		const std::string UsedLang = GetValidatedLanguageStr(InLanguage);
		if (UsedLang.empty())
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Task failed. Result: Invalid language"), *FString(__func__));
			return false;
		}

		UE_LOG(LogAzSpeech, Display, TEXT("%s: Using language: %s"), *FString(__func__), *FString(UTF8_TO_TCHAR(UsedLang.c_str())));

		InConfig->SetSpeechRecognitionLanguage(UsedLang);

		return true;
	}

	bool AddSynthesizerSDKSettings(const std::shared_ptr<SpeechConfig>& InConfig, const std::string& InLanguage, const std::string& InVoiceName, bool& bIsUsingAutoLang)
	{
		if (!AddSharedSDKSettings(InConfig, InLanguage, bIsUsingAutoLang))
		{
			return false;
		}

		InConfig->SetProperty("SpeechSynthesis_KeepConnectionAfterStopping", "false");

		if (bIsUsingAutoLang)
		{
			return true;
		}

		const std::string UsedLang = GetValidatedLanguageStr(InLanguage);
		const std::string UsedVoice = GetValidatedVoiceNameStr(InVoiceName);

		if (UsedVoice.empty() || UsedVoice.empty())
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Task failed. Result: Invalid language or voice name"), *FString(__func__));
			return false;
		}

		UE_LOG(LogAzSpeech, Display, TEXT("%s: Using language: %s"), *FString(__func__), *FString(UTF8_TO_TCHAR(UsedLang.c_str())));
		InConfig->SetSpeechSynthesisLanguage(UsedLang);

		UE_LOG(LogAzSpeech, Display, TEXT("%s: Using voice: %s"), *FString(__func__), *FString(UTF8_TO_TCHAR(UsedVoice.c_str())));
		InConfig->SetSpeechSynthesisVoiceName(UsedVoice);

		return true;
	}

	std::shared_ptr<SpeechSynthesizer> GetAzureSynthesizer(const std::shared_ptr<AudioConfig>& InAudioConfig = AudioConfig::FromDefaultSpeakerOutput(), const std::string& InLanguage = "Default", const std::string& InVoiceName = "Default")
	{
		if (!CheckAzSpeechSettings())
		{
			return nullptr;
		}

		const auto Settings = GetAzSpeechKeys();
		const auto SpeechConfig = SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));
				
		bool bIsUsingAutoLang = false;
		if (!AddSynthesizerSDKSettings(SpeechConfig, InLanguage, InVoiceName, bIsUsingAutoLang))
		{
			return nullptr;
		}
				
		if (bIsUsingAutoLang)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("%s: Initializing language auto detection..."), *FString(__func__));
			UE_LOG(LogAzSpeech, Warning, TEXT("AzSpeech - %s: Note: Synthesizers currently only support language detection from open range"), *FString(__func__));

			return SpeechSynthesizer::FromConfig(SpeechConfig, AutoDetectSourceLanguageConfig::FromOpenRange(), InAudioConfig);
		}

		return SpeechSynthesizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	std::shared_ptr<SpeechRecognizer> GetAzureRecognizer(const std::shared_ptr<AudioConfig>& InAudioConfig = AudioConfig::FromDefaultMicrophoneInput(), const std::string& InLanguage = "Default")
	{
		if (!CheckAzSpeechSettings())
		{
			return nullptr;
		}
		
		const auto Settings = GetAzSpeechKeys();
		const auto SpeechConfig = SpeechConfig::FromSubscription(Settings.at(0), Settings.at(1));

		bool bIsUsingAutoLang = false;
		if (!AddRecognitionSDKSettings(SpeechConfig, InLanguage, bIsUsingAutoLang))
		{
			return nullptr;
		}

		if (bIsUsingAutoLang)
		{
			const std::vector<std::string> Candidates = GetCandidateLanguages();
			if (Candidates.empty())
			{
				UE_LOG(LogAzSpeech, Error, TEXT("%s: Task failed. Result: Invalid candidate languages"), *FString(__func__));

				return nullptr;
			}

			UE_LOG(LogAzSpeech, Display, TEXT("%s: Initializing language auto detection"), *FString(__func__));
			for (const std::string& Iterator : Candidates)
			{
				UE_LOG(LogAzSpeech, Display, TEXT("%s: Candidate: %s"), *FString(__func__), *FString(UTF8_TO_TCHAR(Iterator.c_str())));
			}

			return SpeechRecognizer::FromConfig(SpeechConfig, AutoDetectSourceLanguageConfig::FromLanguages(Candidates), InAudioConfig);
		}

		return SpeechRecognizer::FromConfig(SpeechConfig, InAudioConfig);
	}

	const FString CancellationReasonToString(const CancellationReason& CancellationReason)
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
				return FString("Undefined");
		}
	}

	void ProcessCancellationError(const CancellationErrorCode& ErrorCode, const std::string& ErrorDetails)
	{
		FString ErrorCodeStr;
		switch (ErrorCode)
		{
			case CancellationErrorCode::NoError:
				ErrorCodeStr = "Error";
				break;

			case CancellationErrorCode::AuthenticationFailure:
				ErrorCodeStr = "AuthenticationFailure";
				break;

			case CancellationErrorCode::BadRequest:
				ErrorCodeStr = "BadRequest";
				break;

			case CancellationErrorCode::TooManyRequests:
				ErrorCodeStr = "TooManyRequests";
				break;

			case CancellationErrorCode::Forbidden:
				ErrorCodeStr = "Forbidden";
				break;

			case CancellationErrorCode::ConnectionFailure:
				ErrorCodeStr = "ConnectionFailure";
				break;

			case CancellationErrorCode::ServiceTimeout:
				ErrorCodeStr = "ServiceTimeout";
				break;

			case CancellationErrorCode::ServiceError:
				ErrorCodeStr = "ServiceError";
				break;

			case CancellationErrorCode::ServiceUnavailable:
				ErrorCodeStr = "ServiceUnavailable";
				break;

			case CancellationErrorCode::RuntimeError:
				ErrorCodeStr = "RuntimeError";
				break;

			case CancellationErrorCode::ServiceRedirectTemporary:
				ErrorCodeStr = "ServiceRedirectTemporary";
				break;

			case CancellationErrorCode::ServiceRedirectPermanent:
				ErrorCodeStr = "ServiceRedirectPermanent";
				break;

			case CancellationErrorCode::EmbeddedModelError:
				ErrorCodeStr = "EmbeddedModelError";
				break;

			default:
				ErrorCodeStr = "Undefined";
				break;
		}

		UE_LOG(LogAzSpeech, Error, TEXT("%s: Error code: %s"), *FString(__func__), *ErrorCodeStr);

		const FString ErrorDetailsStr = UTF8_TO_TCHAR(ErrorDetails.c_str());
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Error Details: %s"), *FString(__func__), *ErrorDetailsStr);		
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Log generated in directory: %s"), *FString(__func__), *GetAzSpeechLogsBaseDir());
	}

	const bool ProcessRecognitionResult(const std::shared_ptr<SpeechRecognitionResult>& Result)
	{
		switch (Result->Reason)
		{
			case ResultReason::RecognizedSpeech:
				UE_LOG(LogAzSpeech, Display, TEXT("%s: Task completed. Reason: RecognizedSpeech"), *FString(__func__));
				return true;

			case ResultReason::RecognizingSpeech:
				UE_LOG(LogAzSpeech, Display, TEXT("%s: Task running. Reason: RecognizingSpeech"), *FString(__func__));
				return true;

			default:
				break;
		}
		
		if (Result->Reason == ResultReason::Canceled)
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Task failed. Reason: Canceled"), *FString(__func__));
			const auto CancellationDetails = CancellationDetails::FromResult(Result);

			UE_LOG(LogAzSpeech, Error, TEXT("%s: Cancellation Reason: %s"), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

			if (CancellationDetails->Reason == CancellationReason::Error)
			{
				ProcessCancellationError(CancellationDetails->ErrorCode, CancellationDetails->ErrorDetails);
			}

			return false;
		}

		UE_LOG(LogAzSpeech, Warning, TEXT("AzSpeech - %s: Undefined reason"), *FString(__func__));
		return false;
	}

	const bool ProcessSynthesisResult(const std::shared_ptr<SpeechSynthesisResult>& Result)
	{
		switch (Result->Reason)
		{
			case ResultReason::SynthesizingAudio :
				UE_LOG(LogAzSpeech, Display, TEXT("%s: Task running. Reason: SynthesizingAudio"), *FString(__func__));
				return true;

			case ResultReason::SynthesizingAudioCompleted:
				UE_LOG(LogAzSpeech, Display, TEXT("%s: Task completed. Reason: SynthesizingAudioCompleted"), *FString(__func__));
				return true;

			case ResultReason::SynthesizingAudioStarted:
				UE_LOG(LogAzSpeech, Display, TEXT("%s: Task started. Reason: SynthesizingAudioStarted"), *FString(__func__));
				return true;

			default: 
				break;
		}

		if (Result->Reason == ResultReason::Canceled)
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Task failed. Reason: Canceled"), *FString(__func__));
			const auto CancellationDetails = SpeechSynthesisCancellationDetails::FromResult(Result);

			UE_LOG(LogAzSpeech, Error, TEXT("%s: Cancellation Reason: %s"), *FString(__func__), *CancellationReasonToString(CancellationDetails->Reason));

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
