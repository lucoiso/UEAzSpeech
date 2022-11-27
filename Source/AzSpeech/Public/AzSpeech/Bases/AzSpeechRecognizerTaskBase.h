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

	/* Task delegate that will be called when completed */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FRecognitionCompletedDelegate RecognitionCompleted;

	/* Task delegate that will be called when dpdated */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FRecognitionUpdatedDelegate RecognitionUpdated;

	/* Task delegate that will be called when started */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FAzSpeechTaskGenericDelegate RecognitionStarted;

	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	void EnableContinuousRecognition();

	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	void DisableContinuousRecognition();	

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FString GetLastRecognizedString() const;
	
protected:
	bool bContinuousRecognition = false;

	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognitionResult> LastRecognitionResult;

	virtual void StopAzureTaskWork() override;

	virtual void ClearAllBindings() override;
	virtual void ConnectTaskSignals() override;

	virtual void ReleaseResources() override;

	virtual void BroadcastFinalResult() override;

	virtual void ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) override;

	virtual void OnRecognitionUpdated();

	void StartRecognitionWork(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);

private:
	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognizer> RecognizerObject;
	bool bRecognizingStatusAlreadyShown = false;

	bool InitializeRecognizer(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);
	const bool ProcessRecognitionResult();
	void DisconnectRecognitionSignals();
};
