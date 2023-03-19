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
	UFUNCTION(BlueprintCallable, Category = "AzSpeech | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Text To .wav File with Default Options"))
	static UTextToWavFileAsync* TextToWavFile_DefaultOptions(UObject* WorldContextObject, const FString& SynthesisText, const FString& FilePath, const FString& FileName, const FString& VoiceName = "Default", const FString& LanguageID = "Default");

	/* Creates a Text-To-WavFile task that will convert your string to a .wav audio file */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Text To .wav File with Custom Options"))
	static UTextToWavFileAsync* TextToWavFile_CustomOptions(UObject* WorldContextObject, const FString& SynthesisText, const FString& FilePath, const FString& FileName, const FAzSpeechSettingsOptions& Options);
};
