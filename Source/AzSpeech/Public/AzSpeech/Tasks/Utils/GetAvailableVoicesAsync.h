// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>
#include "GetAvailableVoicesAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAzSpeechFindAvailableVoicesFailDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAzSpeechFindAvailableVoicesResultDelegate, const TArray<FString>&, AvailableVoices);

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UGetAvailableVoicesAsync : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    /* Task delegate that will be called when the available voices are returned by Azure */
    UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
    FAzSpeechFindAvailableVoicesResultDelegate Success;

    /* Task delegate that will be called when the task fails to get the available voices */
    UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
    FAzSpeechFindAvailableVoicesFailDelegate Fail;

    /* Get the available synthesis voices */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Get Available Voices Async"))
    static UGetAvailableVoicesAsync* GetAvailableVoicesAsync(UObject* const WorldContextObject, const FString& Locale = "");

    virtual void Activate() override;
    virtual void SetReadyToDestroy() override;

protected:
    FName TaskName;
    FString Locale;

private:
    void BroadcastResult(const TArray<FString>& Result);
    const TArray<FString> GetAvailableVoices() const;
};