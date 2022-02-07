#pragma once

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx.h>
THIRD_PARTY_INCLUDES_END

/**
 *
 */
namespace FAzureSpeechWrapper
{
	static wchar_t* DoVoiceToTextWork(const std::string SubscriptionID, const std::string RegionID,
	                                  const std::string LanguageID)
	{
		if (SubscriptionID.size() == 0 || RegionID.size() == 0 || LanguageID.size() == 0)
		{
			return new wchar_t;
		}

		const auto Config = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(
			SubscriptionID, RegionID);
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

		const std::string Locale = setlocale(LC_ALL, "");
		const char* SrcBuf = RecognizedString.c_str();
		const size_t Size = strlen(SrcBuf) + 1;
		wchar_t* DstBuf = new wchar_t[Size];
		size_t OutSize;
		mbstowcs_s(&OutSize, DstBuf, Size, SrcBuf, Size - 1);
		setlocale(LC_ALL, Locale.c_str());

		return DstBuf;
	}

	static bool DoTextToVoiceWork(const std::string TextToConvert, const std::string SubscriptionID,
	                              const std::string RegionID, const std::string LanguageID,
	                              const std::string VoiceName)
	{
		if (SubscriptionID.size() == 0 || RegionID.size() == 0 || LanguageID.size() == 0 || TextToConvert.size() == 0)
		{
			return false;
		}

		const auto SpeechConfig = Microsoft::CognitiveServices::Speech::SpeechConfig::FromSubscription(
			SubscriptionID, RegionID);
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
}
