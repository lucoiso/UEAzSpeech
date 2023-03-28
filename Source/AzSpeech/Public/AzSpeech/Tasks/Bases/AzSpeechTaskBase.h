// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "AzSpeech/Structures/AzSpeechSettingsOptions.h"
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
	const FName GetTaskName() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FAzSpeechSubscriptionOptions GetSubscriptionOptions() const;

	virtual void SetReadyToDestroy() override;

protected:
	TSharedPtr<class FAzSpeechRunnableBase> RunnableTask;
	FName TaskName = NAME_None;

	FAzSpeechSubscriptionOptions SubscriptionOptions;

	const UObject* WorldContextObject;
	bool bIsSSMLBased = false;

	virtual bool StartAzureTaskWork();
	virtual void BroadcastFinalResult();

	mutable FCriticalSection Mutex;

#if WITH_EDITOR
	virtual void PrePIEEnded(bool bIsSimulating);

	bool bEndingPIE = false;
#endif

	template <typename ReturnTy, typename ResultType>
	constexpr ReturnTy GetProperty(const ResultType& Result, const Microsoft::CognitiveServices::Speech::PropertyId& ID)
	{
		const auto Property = Result->Properties.GetProperty(ID);
		if (Property.empty())
		{
			return ReturnTy();
		}

		if constexpr (std::is_same_v<ReturnTy, FString>)
		{
			return FString(Property.c_str());
		}
		else if constexpr (std::is_same_v<ReturnTy, FName>)
		{
			return FName(Property.c_str());
		}
		else if constexpr (std::is_same_v<ReturnTy, int32>)
		{
			return FCString::Atoi(*FString(Property.c_str()));
		}
		else if constexpr (std::is_same_v<ReturnTy, float>)
		{
			return FCString::Atof(*FString(Property.c_str()));
		}
		else if constexpr (std::is_same_v<ReturnTy, bool>)
		{
			return FCString::ToBool(Property.c_str());
		}

		return ReturnTy();
	}

private:
	bool bIsTaskActive = false;
	bool bIsReadyToDestroy = false;
};

UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UAzSpeechTaskStatus final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static bool IsTaskActive(const UAzSpeechTaskBase* Test);

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static bool IsTaskReadyToDestroy(const UAzSpeechTaskBase* Test);	

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static bool IsTaskStillValid(const UAzSpeechTaskBase* Test);
};