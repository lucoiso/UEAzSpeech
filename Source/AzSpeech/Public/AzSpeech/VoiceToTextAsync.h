// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechRecognizerTaskBase.h"
#include "VoiceToTextAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UVoiceToTextAsync : public UAzSpeechRecognizerTaskBase
{
	GENERATED_BODY()

public:
	/* Creates a Voice-To-Text task that will convert your speech to string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UVoiceToTextAsync* VoiceToText(const UObject* WorldContextObject, const FString& LanguageId = "Default", const bool bContinuosRecognition = false);

	virtual void Activate() override;

protected:
	virtual bool StartAzureTaskWork_Internal() override;
	
	const UObject* WorldContextObject;
	FString LanguageID;
};
