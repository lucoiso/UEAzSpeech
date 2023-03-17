// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechWavFileSynthesisBase.h"
#include "TextToWavFileAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UTextToWavFileAsync : public UAzSpeechWavFileSynthesisBase
{
	GENERATED_BODY()

public:
	/* Creates a Text-To-WavFile task that will convert your string to a .wav audio file */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Text To .wav File", DeprecatedFunction = "true", DeprecationMessage = "Use the version with the new options structure instead."))
	static FORCEINLINE UTextToWavFileAsync* TextToWavFile(UObject* WorldContextObject, const FString& SynthesisText, const FString& FilePath, const FString& FileName, const FString& VoiceName = "Default", const FString& LanguageID = "Default")
	{
		return TextToWavFileAsync(WorldContextObject, SynthesisText, FilePath, FileName, FAzSpeechSettingsOptions(LanguageID, VoiceName));
	}


	/* Creates a Text-To-WavFile task that will convert your string to a .wav audio file */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Text To .wav File Async", AutoCreateRefTerm = "Options"))
	static UTextToWavFileAsync* TextToWavFileAsync(UObject* WorldContextObject, const FString& SynthesisText, const FString& FilePath, const FString& FileName, const FAzSpeechSettingsOptions& Options  = FAzSpeechSettingsOptions());
};
