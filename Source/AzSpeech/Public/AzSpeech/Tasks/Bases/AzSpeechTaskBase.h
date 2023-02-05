// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_audio_config.h>
THIRD_PARTY_INCLUDES_END

#include "AzSpeechTaskBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAzSpeechTaskGenericDelegate);

/**
 *
 */
UCLASS(Abstract, NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UAzSpeechTaskBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	friend class FAzSpeechRunnableBase;

public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (DisplayName = "Stop AzSpeech Task"))
	virtual void StopAzSpeechTask();

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	bool IsTaskActive() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	bool IsTaskReadyToDestroy() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (DefaultToSelf = "Test"))
	static const bool IsTaskStillValid(const UAzSpeechTaskBase* Test);

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const bool IsUsingAutoLanguage() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FString GetTaskName() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FString GetLanguageID() const;

	virtual void SetReadyToDestroy() override;

protected:
	TSharedPtr<class FAzSpeechRunnableBase> RunnableTask;
	FName TaskName = NAME_None;

	FString LanguageID;
	const UObject* WorldContextObject;
	bool bIsSSMLBased = false;

	virtual bool StartAzureTaskWork();
	virtual void BroadcastFinalResult();

	mutable FCriticalSection Mutex;

#if WITH_EDITOR
	virtual void PrePIEEnded(bool bIsSimulating);

	bool bEndingPIE = false;
#endif

	template<typename ...Args>
	constexpr const bool HasEmptyParameters(Args&& ...args) const
	{
		const bool bOutput = AzSpeech::Internal::HasEmptyParam(std::forward<Args>(args)...);
		if (bOutput)
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: Missing parameters"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
		}

		return bOutput;
	}

private:
	bool bIsTaskActive = false;
	bool bIsReadyToDestroy = false;

	void ValidateLanguageID();
};
