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

	/* Creates a SSML-To-Stream task that will convert your SSML file to a audio data stream */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
			DisplayName = "SSML To Stream Async", DeprecatedFunction = "true", DeprecationMessage =
			"Use 'SSML To Stream' instead: AzSpeechData will be replaced by AzSpeech Settings (Project Settings)"))
	static USSMLToStreamAsync* SSMLToStreamAsync(const UObject* WorldContextObject,
	                                             const FString& SSMLString,
	                                             const FAzSpeechData Parameters)
	{
		return SSMLToStream(WorldContextObject, SSMLString);
	};

	/* Creates a SSML-To-Stream task that will convert your SSML file to a audio data stream */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Stream"))
	static USSMLToStreamAsync* SSMLToStream(const UObject* WorldContextObject, const FString& SSMLString);

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FString SSMLString;
};
