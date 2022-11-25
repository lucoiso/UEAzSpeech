// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/Bases/AzSpeechAudioDataSynthesisBase.h"
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

	virtual void StopAzSpeechTask() override;

protected:
	virtual void OnSynthesisUpdate() override;
	
private:
	TWeakObjectPtr<UAudioComponent> AudioComponent;
};
