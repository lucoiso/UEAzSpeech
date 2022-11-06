// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechSynthesizerTaskBase.h"
#include "SSMLToWavAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API USSMLToWavAsync final : public UAzSpeechSynthesizerTaskBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FBooleanSynthesisDelegate SynthesisCompleted;

	/* Creates a Text-To-Wav task that will convert your string to a .wav audio file */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "SSML To WAV"))
	static USSMLToWavAsync* SSMLToWav(const UObject* WorldContextObject, const FString& SSMLString, const FString& FilePath, const FString& FileName);

	virtual void Activate() override;

protected:
	virtual bool StartAzureTaskWork_Internal() override;

private:
	const UObject* WorldContextObject;
	FString SSMLString;
	FString FilePath;
	FString FileName;
	
	bool DoAzureTaskWork_Internal(const std::string& InSSML, const std::string& InFilePath);
};
