// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
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
	
	const std::shared_ptr<Microsoft::CognitiveServices::Speech::Recognizer> GetRecognizer() const;
	
	// FRunnable interface
	virtual uint32 Run() override;
	virtual void Stop() override;
	// End of FRunnable interface

private:
	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognizer> SpeechRecognizer;

protected:
	virtual void ClearSignals() override;
	virtual void RemoveBindings() override;

	virtual bool ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) const override;

	virtual bool InitializeAzureObject() override;

private:
	bool ConnectRecognitionSignals();	
	bool ProcessRecognitionResult(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognitionResult>& LastResult);
};
