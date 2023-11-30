// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>
#include "RecognitionMapCheckAsync.generated.h"

struct FAzSpeechRecognitionMap;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAzSpeechMapCheckDelegate_Generic);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAzSpeechMapCheckDelegate_WithValue, const int32, RecognitionResult);

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API URecognitionMapCheckAsync : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    /* Task delegate that will be called when a value is found in the recognition data */
    UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
    FAzSpeechMapCheckDelegate_WithValue Found;

    /* Task delegate that will be called when the task doesn't found any value that match the recognition data in the given group */
    UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
    FAzSpeechMapCheckDelegate_Generic NotFound;

    /* Search in the recognition map for the key that best matches with the input string and return the registered value (See Project Settings -> AzSpeech: Recognition Map) */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Check Return from Recognition Map"))
    static URecognitionMapCheckAsync* RecognitionMapCheckAsync(UObject* const WorldContextObject, const FString& InString, const FName& GroupName, const bool bStopAtFirstTrigger = false);

    virtual void Activate() override;
    virtual void SetReadyToDestroy() override;

protected:
    FName TaskName;
    FName GroupName;
    FString InputString;
    bool bStopAtFirstTrigger;

private:
    void BroadcastResult(const int32 Result);
    const int32 CheckRecognitionResult() const;
    const bool CheckStringContains(const FString& KeyType, const FString& Key) const;

    const FName GetStringDelimiters() const;
    const bool CheckStringDelimiters(const int32 Index) const;

    const FAzSpeechRecognitionMap GetRecognitionMap(const FName& InGroup) const;
};