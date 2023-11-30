// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeech/Tasks/Bases/AzSpeechTaskBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_recognition_base_async_recognizer.h>
#include <speechapi_cxx_speech_recognition_result.h>
THIRD_PARTY_INCLUDES_END

#include "AzSpeechRecognizerTaskBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRecognitionUpdatedDelegate, const FString, UpdatedString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRecognitionCompletedDelegate, const FString, FinalString);

/**
 *
 */
UCLASS(Abstract, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class AZSPEECH_API UAzSpeechRecognizerTaskBase : public UAzSpeechTaskBase
{
    GENERATED_BODY()

    friend class FAzSpeechRecognitionRunnableBase;

public:
    /* Task delegate that will be called when completed */
    UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
    FRecognitionCompletedDelegate RecognitionCompleted;

    /* Task delegate that will be called when dpdated */
    UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
    FRecognitionUpdatedDelegate RecognitionUpdated;

    /* Task delegate that will be called when started */
    UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
    FAzSpeechTaskGenericDelegate RecognitionStarted;

    /* Task delegate that will be called when failed */
    UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
    FAzSpeechTaskGenericDelegate RecognitionFailed;

    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    const FAzSpeechRecognitionOptions& GetRecognitionOptions() const;

    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    void SetRecognitionOptions(const FAzSpeechRecognitionOptions& Options);

    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    const FString GetRecognizedString() const;

    /* Get Recognition Duration in Milliseconds */
    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    const int64 GetRecognitionDuration() const;

    UFUNCTION(BlueprintPure, Category = "AzSpeech")
    const int32 GetRecognitionLatency() const;

protected:
    FName PhraseListGroup = NAME_None;
    FAzSpeechRecognitionOptions RecognitionOptions;

    virtual void StartRecognitionWork();

    virtual void BroadcastFinalResult() override;
    virtual void OnRecognitionUpdated(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognitionResult>& LastResult);

private:
    std::string RecognizedText;

    int64 RecognitionDuration = 0;
    int32 RecognitionLatency = 0;
};
