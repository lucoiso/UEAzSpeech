// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_embedded_speech_config.h>
#include <speechapi_cxx_hybrid_speech_config.h>
#include <speechapi_cxx_speech_config.h>
THIRD_PARTY_INCLUDES_END

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
	FString LanguageId;
	const UObject* WorldContextObject;

	virtual bool StartAzureTaskWork_Internal();
	virtual void SetReadyToDestroy() override;

	virtual void ApplyExtraSettings() {};
	virtual void ClearBindings() {};

	const bool IsUsingAutoLanguage() const;

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

	template<typename Ty>
	const bool HasEmptyParam(const Ty& Arg1) const
	{
		return Arg1.IsEmpty();
	}

	template<typename Ty, typename ...Args>
	const bool HasEmptyParam(const Ty& Arg1, Args&& ...args) const
	{
		const bool bOutput = HasEmptyParam(Arg1) || HasEmptyParam(std::forward<Args>(args)...);
		if (bOutput)
		{
			UE_LOG(LogAzSpeech, Error, TEXT("%s: Missing parameters!"), *FString(__func__));
		}

		return bOutput;
	}

	virtual void ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig);
	void EnableLogInConfiguration(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InSpeechConfig);

	const FString CancellationReasonToString(const Microsoft::CognitiveServices::Speech::CancellationReason& CancellationReason) const;
	void ProcessCancellationError(const Microsoft::CognitiveServices::Speech::CancellationErrorCode& ErrorCode, const std::string& ErrorDetails) const;

	static std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig> CreateSpeechConfig();

private:
	bool bIsReadyToDestroy = false;
	bool bHasStopped = false;
};
