// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/AzSpeechRecognizerTaskBase.h"
#include "VoiceToTextAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceToTextDelegate, const FString&, RecognizedString);

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UVoiceToTextAsync final : public UAzSpeechRecognizerTaskBase
{
	GENERATED_BODY()

public:
	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FVoiceToTextDelegate TaskCompleted;

	/* Creates a Voice-To-Text task that will convert your speech to string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UVoiceToTextAsync* VoiceToText(const UObject* WorldContextObject, const FString& LanguageId = "Default");

	virtual void Activate() override;

protected:
	virtual bool StartAzureTaskWork_Internal() override;

private:
	const UObject* WorldContextObject;
	FString LanguageID;
	
	std::string DoAzureTaskWork_Internal(const std::string& InLanguageID);
};
