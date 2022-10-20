// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechTaskBase.h"
#include "WavToTextAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWavToTextDelegate, const FString&, RecognizedString);

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UWavToTextAsync final : public UAzSpeechTaskBase
{
	GENERATED_BODY()

	// Classes used for testing: https://github.com/lucoiso/UEAzSpeech_Tests
	friend class FWavToTextDefaultTest;
	friend class FWavToTextAutoTest;

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FWavToTextDelegate TaskCompleted;

	/* Creates a Wav-To-Text task that will convert your Wav file to string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "WAV To Text"))
	static UWavToTextAsync* WavToText(const UObject* WorldContextObject, const FString& FilePath, const FString& FileName, const FString& LanguageId = "Default");

	virtual void Activate() override;

protected:
	virtual void StartAzureTaskWork_Internal() override;

private:
	const UObject* WorldContextObject;
	FString LanguageID;
	FString FilePath;
	FString FileName;
	
	std::string DoAzureTaskWork_Internal(const std::string& InFilePath, const std::string& InLanguageID);
};
