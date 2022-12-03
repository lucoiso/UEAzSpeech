// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeechAudioDataSynthesisBase.generated.h"

/**
 *
 */
UCLASS(Abstract, MinimalAPI, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class UAzSpeechAudioDataSynthesisBase : public UAzSpeechSynthesizerTaskBase
{
	GENERATED_BODY()
	
protected:
	virtual bool StartAzureTaskWork() override;
};
