// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechSpeechSynthesisBase.h"
#include "TextToSpeechAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UTextToSpeechAsync : public UAzSpeechSpeechSynthesisBase
{
	GENERATED_BODY()

public:
	/* Creates a Text-To-Speech task that will convert your text to speech */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DeprecatedFunction = "true", DeprecationMessage = "Use the version with the new options structure instead"))
	static FORCEINLINE UTextToSpeechAsync* TextToSpeech(UObject* WorldContextObject, const FString& SynthesisText, const FString& VoiceName = "Default", const FString& LanguageID = "Default")
	{
		return TextToSpeechAsync(WorldContextObject, SynthesisText, FAzSpeechSettingsOptions(*LanguageID, *VoiceName));
	}

	/* Creates a Text-To-Speech task that will convert your text to speech */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Options"))
	static UTextToSpeechAsync* TextToSpeechAsync(UObject* WorldContextObject, const FString& SynthesisText, const FAzSpeechSettingsOptions& Options = FAzSpeechSettingsOptions());
};
