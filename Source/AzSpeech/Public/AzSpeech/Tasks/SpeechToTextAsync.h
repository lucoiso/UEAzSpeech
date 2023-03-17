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
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DeprecatedFunction = "true", DeprecationMessage = "Use the version with the new options structure instead."))
	static FORCEINLINE USpeechToTextAsync* SpeechToText(UObject* WorldContextObject, const FString& LanguageID = "Default", const FString& AudioInputDeviceID = "Default", const FName PhraseListGroup = NAME_None)
	{
		return SpeechToTextAsync(WorldContextObject, AudioInputDeviceID, PhraseListGroup, FAzSpeechSettingsOptions(*LanguageID));
	}

	/* Creates a Speech-To-Text task that will convert your speech to string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Options"))
	static USpeechToTextAsync* SpeechToTextAsync(UObject* WorldContextObject, const FString& AudioInputDeviceID = "Default", const FName PhraseListGroup = NAME_None, const FAzSpeechSettingsOptions& Options = FAzSpeechSettingsOptions());

	virtual void Activate() override;
	
	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	bool IsUsingDefaultAudioInputDevice() const;

protected:
	virtual bool StartAzureTaskWork() override;
	
private:
	FString AudioInputDeviceID;
};
