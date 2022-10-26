// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechSynthesizerTaskBase.h"
#include "SSMLToVoiceAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSSMLToVoiceDelegate, const bool, OutputValue);

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToVoiceAsync final : public UAzSpeechSynthesizerTaskBase
{
	GENERATED_BODY()

	// Class used for testing: https://github.com/lucoiso/UEAzSpeech_Tests
	friend class FSSMLToVoiceTest;

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FSSMLToVoiceDelegate TaskCompleted;

	/* Creates a SSML-To-Voice task that will convert your SSML file to speech */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To Voice"))
	static USSMLToVoiceAsync* SSMLToVoice(const UObject* WorldContextObject, const FString& SSMLString);

	virtual void Activate() override;

protected:
	virtual bool StartAzureTaskWork_Internal() override;

private:
	const UObject* WorldContextObject;
	FString SSMLString;
	
	bool DoAzureTaskWork_Internal(const std::string& InSSML);
};
