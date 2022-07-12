// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechData.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SSMLToWavAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToWavAsync final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FSSMLToWavDelegate TaskCompleted;

	/* Creates a Text-To-Wav task that will convert your string into a .wav audio file */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech",
		meta = (BlueprintInternalUseOnly = "true",
			WorldContext = "WorldContextObject", DisplayName = "SSML To WAV Async"))
	static USSMLToWavAsync* SSMLToWavAsync(const UObject* WorldContextObject,
	                                       const FString& SSMLString,
	                                       const FString& FilePath,
	                                       const FString& FileName,
	                                       const FAzSpeechData Parameters);

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FString SSMLString;
	FString FilePath;
	FString FileName;
	FAzSpeechData Parameters;
};
