// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechSynthesizerTaskBase.h"
#include "TextToStreamAsync.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UTextToStreamAsync final : public UAzSpeechSynthesizerTaskBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FStreamSynthesisDelegate SynthesisCompleted;

	/* Creates a Text-To-Stream task that will convert your text to a audio data stream */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UTextToStreamAsync* TextToStream(const UObject* WorldContextObject, const FString& TextToConvert, const FString& VoiceName = "Default", const FString& LanguageId = "Default");

	virtual void Activate() override;

protected:
	virtual bool StartAzureTaskWork_Internal() override;
	virtual void OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs) override;

private:
	const UObject* WorldContextObject;
	FString TextToConvert;
	FString VoiceName;
	FString LanguageID;
};
