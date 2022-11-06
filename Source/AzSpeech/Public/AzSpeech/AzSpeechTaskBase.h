// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AzSpeechTaskBase.generated.h"

/**
 *
 */
UCLASS(Abstract, MinimalAPI, NotPlaceable, Category = "AzSpeech")
class UAzSpeechTaskBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (DisplayName = "Stop AzSpeech Task"))
	virtual void StopAzSpeechTask();

protected:
	virtual bool StartAzureTaskWork_Internal();

	virtual bool CanBroadcast() const;

private:
#if WITH_EDITOR
	void OnEndPIE(const bool bIsSimulating);
#endif
};
