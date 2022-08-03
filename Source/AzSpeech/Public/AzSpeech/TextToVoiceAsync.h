// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechData.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "TextToVoiceAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UTextToVoiceAsync final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FTextToVoiceDelegate TaskCompleted;

	/* Creates a Text-To-Voice task that will convert your text to speech */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
			DeprecatedFunction = "true", DeprecationMessage =
			"Use 'Text To Voice' instead - AzSpeechData will be replaced by AzSpeech Settings (Project Settings)"
		))
	static UTextToVoiceAsync* TextToVoiceAsync(const UObject* WorldContextObject,
	                                           const FString& TextToConvert,
	                                           const FString& VoiceName,
	                                           const FAzSpeechData Parameters)
	{
		return TextToVoice(WorldContextObject, TextToConvert, VoiceName, Parameters.LanguageID);
	}

	/* Creates a Text-To-Voice task that will convert your text to speech */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UTextToVoiceAsync* TextToVoice(const UObject* WorldContextObject,
	                                      const FString& TextToConvert,
	                                      const FString& VoiceName = "Default",
	                                      const FString& LanguageId = "Default");

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FString TextToConvert;
	FString VoiceName;
	FString LanguageID;
};
