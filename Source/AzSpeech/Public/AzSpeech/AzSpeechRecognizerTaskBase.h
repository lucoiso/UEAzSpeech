// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechTaskBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx.h>
THIRD_PARTY_INCLUDES_END

#include "AzSpeechRecognizerTaskBase.generated.h"

/**
 *
 */
UCLASS(Abstract, MinimalAPI, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class UAzSpeechRecognizerTaskBase : public UAzSpeechTaskBase
{
	GENERATED_BODY()

public:	
	virtual void Activate() override;
	
protected:
	virtual bool StartAzureTaskWork_Internal() override;
	
	std::shared_ptr<class Microsoft::CognitiveServices::Speech::SpeechRecognizer> RecognizerObject;
};
