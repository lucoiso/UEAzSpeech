// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "AzSpeech/AzSpeechSettings.h"
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
	friend class UAzSpeechTaskStatus;

public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (DisplayName = "Stop AzSpeech Task"))
	virtual void StopAzSpeechTask();

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const bool IsUsingAutoLanguage() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FName GetTaskName() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FAzSpeechSettingsOptions GetTaskOptions() const;

	virtual void SetReadyToDestroy() override;

protected:
	TSharedPtr<class FAzSpeechRunnableBase> RunnableTask;
	FName TaskName = NAME_None;
	FAzSpeechSettingsOptions TaskOptions;

	const UObject* WorldContextObject;
	bool bIsSSMLBased = false;

	virtual bool StartAzureTaskWork();
	virtual void BroadcastFinalResult();

	mutable FCriticalSection Mutex;

#if WITH_EDITOR
	virtual void PrePIEEnded(bool bIsSimulating);

	bool bEndingPIE = false;
#endif

	static FAzSpeechSettingsOptions GetValidatedOptions(const FAzSpeechSettingsOptions& Options);

private:
	bool bIsTaskActive = false;
	bool bIsReadyToDestroy = false;

	static FName GetValidatedLanguageID(const FName& Language);
	static FName GetValidatedVoiceName(const FName& Voice);
};

UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UAzSpeechTaskStatus final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static FORCEINLINE bool IsTaskActive(const UAzSpeechTaskBase* Test)
	{
		return IsValid(Test) && Test->bIsTaskActive;
	}

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static FORCEINLINE bool IsTaskReadyToDestroy(const UAzSpeechTaskBase* Test)
	{
		return IsValid(Test) && Test->bIsReadyToDestroy;
	}

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static FORCEINLINE bool IsTaskStillValid(const UAzSpeechTaskBase* Test)
	{
		bool bOutput = IsValid(Test) && !IsTaskReadyToDestroy(Test);

#if WITH_EDITOR
		bOutput = bOutput && !Test->bEndingPIE;
#endif

		return bOutput;
	}
};