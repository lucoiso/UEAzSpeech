// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechSynthesizerTaskBase.h"
#include "TextToWavAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UTextToWavAsync : public UAzSpeechSynthesizerTaskBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FBooleanSynthesisDelegate SynthesisCompleted;

	/* Creates a Text-To-Wav task that will convert your string to a .wav audio file */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Text To WAV"))
	static UTextToWavAsync* TextToWav(const UObject* WorldContextObject, const FString& TextToConvert, const FString& FilePath, const FString& FileName, const FString& VoiceName = "Default", const FString& LanguageId = "Default");

	virtual void Activate() override;

protected:
	virtual bool StartAzureTaskWork_Internal() override;
	virtual void OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs) override;

	FString FilePath;
	FString FileName;
};
