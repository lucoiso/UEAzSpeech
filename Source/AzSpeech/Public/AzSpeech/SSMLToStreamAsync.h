// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechData.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SSMLToStreamAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToStreamAsync final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FSSMLToStreamDelegate TaskCompleted;

	/* Creates a SSML-To-Stream task that will convert your SSML file into a audio data stream */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech",
		meta = (BlueprintInternalUseOnly = "true",
			WorldContext = "WorldContextObject", DisplayName = "SSML To Stream Async"))
	static USSMLToStreamAsync* SSMLToStreamAsync(const UObject* WorldContextObject,
	                                             const FString& SSMLString,
	                                             const FAzSpeechData Parameters);

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FString SSMLString;
	FAzSpeechData Parameters;
};
