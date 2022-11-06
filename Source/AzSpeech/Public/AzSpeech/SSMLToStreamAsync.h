// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechSynthesizerTaskBase.h"
#include "SSMLToStreamAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToStreamAsync final : public UAzSpeechSynthesizerTaskBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FStreamSynthesisDelegate SynthesisCompleted;

	/* Creates a SSML-To-Stream task that will convert your SSML file to a audio data stream */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Stream"))
	static USSMLToStreamAsync* SSMLToStream(const UObject* WorldContextObject, const FString& SSMLString);

	virtual void Activate() override;
	
protected:
	virtual bool StartAzureTaskWork_Internal() override;

private:
	const UObject* WorldContextObject;
	FString SSMLString;
	
	std::vector<uint8_t> DoAzureTaskWork_Internal(const std::string& InSSML);
};
