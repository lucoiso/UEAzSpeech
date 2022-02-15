// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechData.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "VoiceToTextAsync.generated.h"

/**
 * 
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UVoiceToTextAsync final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FVoiceToTextDelegate TaskCompleted;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "AzSpeech")
	static UVoiceToTextAsync* VoiceToTextAsync(const UObject* WorldContextObject, FAzSpeechData Parameters);

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FAzSpeechData Parameters;
};