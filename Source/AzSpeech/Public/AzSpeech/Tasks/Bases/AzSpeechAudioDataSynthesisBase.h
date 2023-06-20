// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeechAudioDataSynthesisBase.generated.h"

/**
 *
 */
UCLASS(Abstract, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class AZSPEECH_API UAzSpeechAudioDataSynthesisBase : public UAzSpeechSynthesizerTaskBase
{
	GENERATED_BODY()

protected:
	virtual bool StartAzureTaskWork() override;

	const UObject* WorldContextObject;
};
