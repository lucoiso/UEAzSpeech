// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechData.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "TextToWavAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UTextToWavAsync final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FTextToWavDelegate TaskCompleted;

	/* Creates a Text-To-Wav task that will convert your string into a .wav audio file */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech",
		meta = (BlueprintInternalUseOnly = "true",
			WorldContext = "WorldContextObject", DisplayName = "Text To WAV Async"))
	static UTextToWavAsync* TextToWavAsync(const UObject* WorldContextObject,
	                                       const FString& TextToConvert,
	                                       const FString& FilePath,
	                                       const FString& FileName,
	                                       const FString& VoiceName,
	                                       const FAzSpeechData Parameters);

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FString TextToConvert;
	FString VoiceName;
	FString FilePath;
	FString FileName;
	FAzSpeechData Parameters;
};
