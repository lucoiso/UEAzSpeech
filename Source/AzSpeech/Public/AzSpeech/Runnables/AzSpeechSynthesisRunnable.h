// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/Runnables/Bases/AzSpeechRunnableBase.h"


THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_speech_synthesizer.h>
THIRD_PARTY_INCLUDES_END

/**
 *
 */
class FAzSpeechSynthesisRunnable : public FAzSpeechRunnableBase
{
public:
	FAzSpeechSynthesisRunnable() = delete;
	FAzSpeechSynthesisRunnable(UAzSpeechTaskBase* InOwningTask, const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);

	// FRunnable interface
	virtual uint32 Run() override;
	virtual void Stop() override;
	// End of FRunnable interface

private:
	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesizer> SpeechSynthesizer;

protected:
	virtual void ClearSignals() override;
	virtual void RemoveBindings() override;

	virtual bool ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) const override;

	virtual bool InitializeAzureObject() override;

private:
	bool ConnectSynthesisSignals();
	bool ProcessSynthesisResult(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult);
};
