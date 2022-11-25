// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/Bases/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeechAudioDataSynthesisBase.generated.h"

/**
 *
 */
UCLASS(Abstract, MinimalAPI, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class UAzSpeechAudioDataSynthesisBase : public UAzSpeechSynthesizerTaskBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;
	virtual void StopAzSpeechTask() override;
	
protected:
	virtual bool StartAzureTaskWork_Internal() override;
	virtual void OnSynthesisUpdate() override;
	virtual void BroadcastFinalResult() override;
};
