// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/TextToStreamAsync.h"
#include "TextToVoiceAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UTextToVoiceAsync : public UTextToStreamAsync
{
	GENERATED_BODY()

public:
	/* Creates a Text-To-Voice task that will convert your text to speech */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UTextToVoiceAsync* TextToVoice(const UObject* WorldContextObject, const FString& TextToConvert, const FString& VoiceName = "Default", const FString& LanguageId = "Default");

	virtual void StopAzSpeechTask() override;

protected:
	virtual void OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs) override;
	
private:
	TWeakObjectPtr<UAudioComponent> AudioComponent;
};
