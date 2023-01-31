// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechRecognizerTaskBase.h"
#include "WavFileToTextAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UWavFileToTextAsync : public UAzSpeechRecognizerTaskBase
{
	GENERATED_BODY()

public:
	/* Creates a WavFile-To-Text task that will convert your Wav file to string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = ".wav File To Text"))
	static UWavFileToTextAsync* WavFileToText(UObject* WorldContextObject, const FString& FilePath, const FString& FileName, const FString& LanguageID = "Default", const FName PhraseListGroup = NAME_None);

	virtual void Activate() override;

protected:
	virtual bool StartAzureTaskWork() override;
	
private:
	FString FilePath;
	FString FileName;
};
