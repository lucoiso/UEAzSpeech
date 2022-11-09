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
class AZSPEECH_API USSMLToStreamAsync : public UAzSpeechSynthesizerTaskBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FStreamSynthesisDelegate SynthesisCompleted;

	/* Creates a SSML-To-Stream task that will convert your SSML file to a audio data stream */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Stream"))
	static USSMLToStreamAsync* SSMLToStream(const UObject* WorldContextObject, const FString& SSMLString);
	
protected:
	virtual bool StartAzureTaskWork_Internal() override;
	virtual void OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs) override;
};
