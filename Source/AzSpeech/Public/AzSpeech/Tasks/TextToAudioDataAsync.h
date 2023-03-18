// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechAudioDataSynthesisBase.h"
#include "TextToAudioDataAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UTextToAudioDataAsync : public UAzSpeechAudioDataSynthesisBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FAudioDataSynthesisDelegate SynthesisCompleted;

	/* Creates a Text-To-AudioData task that will convert your text to a audio data stream */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DeprecatedFunction = "true", DeprecationMessage = "Use the version with the new options structure instead"))
	static FORCEINLINE UTextToAudioDataAsync* TextToAudioData(UObject* WorldContextObject, const FString& SynthesisText, const FString& VoiceName = "Default", const FString& LanguageID = "Default")
	{
		return TextToAudioDataAsync(WorldContextObject, SynthesisText, FAzSpeechSettingsOptions(*LanguageID, *VoiceName));
	}

	/* Creates a Text-To-AudioData task that will convert your text to a audio data stream */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UTextToAudioDataAsync* TextToAudioDataAsync(UObject* WorldContextObject, const FString& SynthesisText, const FAzSpeechSettingsOptions& Options);

protected:
	virtual void BroadcastFinalResult() override;
};
