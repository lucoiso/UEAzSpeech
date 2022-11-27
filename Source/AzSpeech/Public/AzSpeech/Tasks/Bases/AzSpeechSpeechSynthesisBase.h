// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "AzSpeech/Tasks/Bases/AzSpeechAudioDataSynthesisBase.h"
#include "AzSpeechSpeechSynthesisBase.generated.h"

class UAudioComponent;

/**
 *
 */
UCLASS(Abstract, MinimalAPI, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class UAzSpeechSpeechSynthesisBase : public UAzSpeechAudioDataSynthesisBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FBooleanSynthesisDelegate SynthesisCompleted;
	
	virtual void SetReadyToDestroy() override;

protected:
	virtual void BroadcastFinalResult() override;
	virtual void OnSynthesisUpdate(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult) override;

	UFUNCTION()
	void OnAudioPlayStateChanged(const EAudioComponentPlayState PlayState);
	
private:
	TWeakObjectPtr<UAudioComponent> AudioComponent;
};
