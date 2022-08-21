// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WavToTextAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWavToTextDelegate, const FString&, RecognizedString);

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
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "WAV To Text"))
	static UWavToTextAsync* WavToText(const UObject* WorldContextObject,
	                                  const FString& FilePath,
	                                  const FString& FileName,
	                                  const FString& LanguageId = "Default");

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FString LanguageID;
	FString FilePath;
	FString FileName;
};
