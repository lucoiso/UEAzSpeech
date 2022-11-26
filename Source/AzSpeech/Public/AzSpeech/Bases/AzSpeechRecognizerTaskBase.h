// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/Bases/AzSpeechTaskBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_speech_recognizer.h>
THIRD_PARTY_INCLUDES_END

#include "AzSpeechRecognizerTaskBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRecognitionUpdatedDelegate, const FString, UpdatedString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRecognitionCompletedDelegate, const FString, FinalString);

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
	FRecognitionCompletedDelegate RecognitionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FRecognitionUpdatedDelegate RecognitionUpdated;

	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	void EnableContinuousRecognition();

	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	void DisableContinuousRecognition();	

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FString GetLastRecognizedString() const;
	
protected:
	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognizer> RecognizerObject;
	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognitionResult> LastRecognitionResult;
	bool bContinuousRecognition = false;

	virtual void ClearBindings() override;
	virtual void ConnectTaskSignals() override;

	virtual void BroadcastFinalResult() override;

	virtual void ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) override;

	virtual void OnRecognitionUpdated();

	bool InitializeRecognizer(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);
	void StartRecognitionWork();

	const bool ProcessRecognitionResult();
	
private:
	bool bRecognizingStatusAlreadyShown = false;
};
