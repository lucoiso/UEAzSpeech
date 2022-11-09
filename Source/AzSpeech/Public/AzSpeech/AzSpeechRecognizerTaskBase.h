// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechTaskBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_speech_recognizer.h>
THIRD_PARTY_INCLUDES_END

#include "AzSpeechRecognizerTaskBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRecognitionUpdated, const FString, UpdatedString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRecognitionCompleted, const FString, FinalString);

/**
 *
 */
UCLASS(Abstract, MinimalAPI, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class UAzSpeechRecognizerTaskBase : public UAzSpeechTaskBase
{
	GENERATED_BODY()

public:	
	virtual void Activate() override;
	virtual void StopAzSpeechTask() override;

	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FRecognitionCompleted RecognitionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FRecognitionUpdated RecognitionUpdated;

	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	void EnableContinuousRecognition();

	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	void DisableContinuousRecognition();	

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FString GetLastRecognizedString() const;
	
protected:
	std::shared_ptr<class Microsoft::CognitiveServices::Speech::SpeechRecognizer> RecognizerObject;
	bool bContinuousRecognition = false;
	
	virtual bool StartAzureTaskWork_Internal() override;

	virtual void ClearBindings() override;
	virtual void ApplyExtraSettings() override;

	virtual void OnRecognitionUpdated(const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& RecognitionEventArgs);
	
private:
	std::string LastRecognizedString;
};
