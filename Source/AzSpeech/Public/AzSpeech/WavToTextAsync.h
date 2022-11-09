// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechRecognizerTaskBase.h"
#include "WavToTextAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UWavToTextAsync : public UAzSpeechRecognizerTaskBase
{
	GENERATED_BODY()

public:
	/* Creates a Wav-To-Text task that will convert your Wav file to string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "WAV To Text"))
	static UWavToTextAsync* WavToText(const UObject* WorldContextObject, const FString& FilePath, const FString& FileName, const FString& LanguageId = "Default", const bool bContinuosRecognition = false);

	virtual void Activate() override;

protected:
	virtual bool StartAzureTaskWork_Internal() override;
	
	const UObject* WorldContextObject;
	FString LanguageID;
	FString FilePath;
	FString FileName;
};
