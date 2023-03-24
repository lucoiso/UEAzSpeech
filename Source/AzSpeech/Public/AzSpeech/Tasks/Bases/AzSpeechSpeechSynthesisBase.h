// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechAudioDataSynthesisBase.h"
#include "AzSpeechSpeechSynthesisBase.generated.h"

class UAudioComponent;

/**
 *
 */
UCLASS(Abstract, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class AZSPEECH_API UAzSpeechSpeechSynthesisBase : public UAzSpeechAudioDataSynthesisBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FBooleanSynthesisDelegate SynthesisCompleted;

	virtual void StopAzSpeechTask() override;
	virtual void SetReadyToDestroy() override;

protected:
	virtual void BroadcastFinalResult() override;

	UFUNCTION()
	void OnAudioPlayStateChanged(const EAudioComponentPlayState PlayState);
	
private:
	TWeakObjectPtr<class UAudioComponent> AudioComponent;
};
