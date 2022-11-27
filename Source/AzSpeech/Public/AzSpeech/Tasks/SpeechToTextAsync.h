// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/Tasks/Bases/AzSpeechRecognizerTaskBase.h"
#include "SpeechToTextAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USpeechToTextAsync : public UAzSpeechRecognizerTaskBase
{
	GENERATED_BODY()

public:
	/* Creates a Speech-To-Text task that will convert your speech to string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static USpeechToTextAsync* SpeechToText(const UObject* WorldContextObject, const FString& LanguageId = "Default", const bool bContinuosRecognition = false);

	virtual void Activate() override;

protected:
	virtual bool StartAzureTaskWork() override;
	
	const UObject* WorldContextObject;
	FString LanguageID;
};
