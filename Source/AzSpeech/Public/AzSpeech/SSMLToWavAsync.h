// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechSynthesizerTaskBase.h"
#include "SSMLToWavAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSSMLToWavDelegate, const bool, OutputValue);

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToWavAsync final : public UAzSpeechSynthesizerTaskBase
{
	GENERATED_BODY()

	// Class used for testing: https://github.com/lucoiso/UEAzSpeech_Tests
	friend class FSSMLToWavTest;

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FSSMLToWavDelegate TaskCompleted;

	/* Creates a Text-To-Wav task that will convert your string to a .wav audio file */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To WAV"))
	static USSMLToWavAsync* SSMLToWav(const UObject* WorldContextObject, const FString& SSMLString, const FString& FilePath, const FString& FileName);

	virtual void Activate() override;

protected:
	virtual void StartAzureTaskWork_Internal() override;

private:
	const UObject* WorldContextObject;
	FString SSMLString;
	FString FilePath;
	FString FileName;
	
	bool DoAzureTaskWork_Internal(const std::string& InSSML, const std::string& InFilePath);
};
