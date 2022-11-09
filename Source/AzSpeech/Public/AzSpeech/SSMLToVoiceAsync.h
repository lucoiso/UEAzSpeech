// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/SSMLToStreamAsync.h"
#include "SSMLToVoiceAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToVoiceAsync : public USSMLToStreamAsync
{
	GENERATED_BODY()

public:
	/* Creates a SSML-To-Voice task that will convert your SSML file to speech */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Voice"))
	static USSMLToVoiceAsync* SSMLToVoice(const UObject* WorldContextObject, const FString& SSMLString);

	virtual void StopAzSpeechTask() override;

protected:
	virtual void OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs) override;

private:
	TWeakObjectPtr<class UAudioComponent> AudioComponent;	
};