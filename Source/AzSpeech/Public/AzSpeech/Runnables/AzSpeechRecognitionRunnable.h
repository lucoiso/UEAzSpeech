// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <vector>
#include <string>
#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_speech_recognizer.h>
THIRD_PARTY_INCLUDES_END

/**
 *
 */
	
class FAzSpeechRecognitionRunnable : public FAzSpeechRunnableBase
{		
public:
	FAzSpeechRecognitionRunnable() = delete;
	FAzSpeechRecognitionRunnable(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);

protected:
	// FRunnable interface
	virtual uint32 Run() override;
	virtual void Exit() override;
	// End of FRunnable interface

private:
	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognizer> SpeechRecognizer;

protected:
	const bool IsSpeechRecognizerValid() const;

	class UAzSpeechRecognizerTaskBase* GetOwningRecognizerTask() const;

	virtual void ClearSignals() override;
	virtual void RemoveBindings() override;

	virtual const bool ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) const override;

	virtual bool InitializeAzureObject() override;

private:
	bool ConnectRecognitionSignals();
	bool InsertPhraseList();

	bool ProcessRecognitionResult(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognitionResult>& LastResult);

	const std::vector<std::string> GetCandidateLanguages() const;
	const TArray<FString> GetPhraseListFromGroup(const FName& InGroup) const;
};
