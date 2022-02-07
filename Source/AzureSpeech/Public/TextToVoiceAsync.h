#pragma once

#include "CoreMinimal.h"
#include "AzureSpeechData.h"
#include "AzureSpeechWrapper.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "TextToVoiceAsync.generated.h"

/**
 * 
 */
UCLASS(NotPlaceable, Category = "AzureSpeech")
class AZURESPEECH_API UTextToVoiceAsync final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "AzureSpeech")
	FTextToVoiceDelegate TaskCompleted;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "AzureSpeech")
	static UTextToVoiceAsync* TextToVoiceAsync(const UObject* WorldContextObject, FString TextToConvert,
	                                           FString VoiceName, FAzureSpeechData Parameters);

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FString TextToConvert;
	FString VoiceName;
	FAzureSpeechData Parameters;
};

namespace AzureSpeech
{
	static void AsyncTextToVoice(FAzureSpeechData Parameters, FString TextToConvert,
	                             FTextToVoiceDelegate Delegate, FString VoiceName)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Parameters, TextToConvert, Delegate, VoiceName]()
		{
			const TFuture<bool> TextToVoiceAsyncWork =
				Async(EAsyncExecution::Thread, [Parameters, TextToConvert, VoiceName]() -> bool
				{
					const std::string SubscriptionID = std::string(TCHAR_TO_UTF8(*Parameters.SubscriptionID));
					const std::string RegionID = std::string(TCHAR_TO_UTF8(*Parameters.RegionID));
					const std::string LanguageID = std::string(TCHAR_TO_UTF8(*Parameters.LanguageID));
					const std::string Name = std::string(TCHAR_TO_UTF8(*VoiceName));
					const std::string ToConvert = std::string(TCHAR_TO_UTF8(*TextToConvert));

					return FAzureSpeechWrapper::DoTextToVoiceWork(ToConvert, SubscriptionID, RegionID, LanguageID,
					                                              Name);
				});

			TextToVoiceAsyncWork.WaitFor(FTimespan::FromSeconds(5));
			const bool bOutputValue = TextToVoiceAsyncWork.Get();

			AsyncTask(ENamedThreads::GameThread, [bOutputValue, Delegate]()
			{
				Delegate.Broadcast(bOutputValue);
			});

			const FString _OutputValueStr = bOutputValue ? "Success" : "Error";

			UE_LOG(LogTemp, Warning,
			       TEXT("AzureSpeech Debug - Subscription: %s, Region: %s, Language: %s, Text To Voice Result: %s"),
			       *FString(Parameters.SubscriptionID), *FString(Parameters.RegionID), *FString(Parameters.LanguageID),
			       *_OutputValueStr);
		});
	}
}
