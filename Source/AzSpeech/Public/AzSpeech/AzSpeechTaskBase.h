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

	static const bool IsTaskStillValid(const UAzSpeechTaskBase* Test);

protected:
	virtual bool StartAzureTaskWork_Internal();
	virtual void SetReadyToDestroy() override;

	virtual void ApplyExtraSettings() {};
	virtual void ClearBindings() {};

	mutable FCriticalSection Mutex;

#if WITH_EDITOR
	virtual void PrePIEEnded(bool bIsSimulating);
#endif

	template<typename SignalTy>
	void SignalDisconecter_T(SignalTy& Signal)
	{
		if (Signal.IsConnected())
		{
			Signal.DisconnectAll();
		}
	};

private:
	bool bIsReadyToDestroy = false;
};
