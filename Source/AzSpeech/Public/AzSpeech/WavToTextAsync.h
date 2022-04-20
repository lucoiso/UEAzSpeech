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
class AZSPEECH_API UWavToTextAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
		FWavToTextDelegate TaskCompleted;

	/* Creates a Wav-To-Text task that will convert your Wav file into string */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "AzSpeech")
		static UWavToTextAsync* WavToTextAsync(const UObject* WorldContextObject, FAzSpeechData Parameters,
			const FString FilePath, const FString FileName);

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FAzSpeechData Parameters;
	FString FilePath;
	FString FileName;
};