// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechData.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SSMLToVoiceAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToVoiceAsync final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FSSMLToVoiceDelegate TaskCompleted;

	/* Creates a SSML-To-Voice task that will convert your SSML file to speech */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech",
		meta = (BlueprintInternalUseOnly = "true",
			WorldContext = "WorldContextObject", DisplayName = "SSML To Voice Async"))
	static USSMLToVoiceAsync* SSMLToVoiceAsync(const UObject* WorldContextObject,
	                                           const FString& SSMLString,
	                                           const FAzSpeechData Parameters);

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FString SSMLString;
	FAzSpeechData Parameters;
};
