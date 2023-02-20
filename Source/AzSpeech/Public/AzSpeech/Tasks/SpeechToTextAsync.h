// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
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
	static USpeechToTextAsync* SpeechToText(UObject* WorldContextObject, const FString& LanguageID = "Default", const FString& AudioInputDeviceID = "Default", const FName PhraseListGroup = NAME_None);

	virtual void Activate() override;
	
	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	bool IsUsingDefaultAudioInputDevice() const;

protected:
	virtual bool StartAzureTaskWork() override;
	
private:
	FString AudioInputDeviceID;
};
