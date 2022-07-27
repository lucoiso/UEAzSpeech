// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechData.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WavToTextAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UWavToTextAsync final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FWavToTextDelegate TaskCompleted;

	/* Creates a Wav-To-Text task that will convert your Wav file to string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
			DisplayName = "WAV To Text Async", DeprecatedFunction, DeprecationMessage =
			"Use 'WAV To Text' instead - AzSpeechData will be replaced by AzSpeech Settings (Project Settings)"))
	static UWavToTextAsync* WavToTextAsync(const UObject* WorldContextObject,
	                                       const FString& FilePath,
	                                       const FString& FileName,
	                                       const FAzSpeechData Parameters)
	{
		return WavToText(WorldContextObject, FilePath, FileName, Parameters.LanguageID);
	}

	/* Creates a Wav-To-Text task that will convert your Wav file to string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "WAV To Text"))
	static UWavToTextAsync* WavToText(const UObject* WorldContextObject,
	                                  const FString& FilePath,
	                                  const FString& FileName,
	                                  const FString& LanguageId = "Default");

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FString LanguageId;
	FString FilePath;
	FString FileName;
};
