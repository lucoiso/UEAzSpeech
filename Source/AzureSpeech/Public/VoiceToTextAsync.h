// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/AzureSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzureSpeechData.h"
#include "AzureSpeechWrapper.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Async/Async.h"
#include "VoiceToTextAsync.generated.h"

/**
 * 
 */
UCLASS(NotPlaceable, Category = "AzureSpeech")
class AZURESPEECH_API UVoiceToTextAsync final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "AzureSpeech")
	FVoiceToTextDelegate TaskCompleted;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "AzureSpeech")
	static UVoiceToTextAsync* VoiceToTextAsync(const UObject* WorldContextObject, FAzureSpeechData Parameters);

	virtual void Activate() override;

private:
	const UObject* WorldContextObject;
	FAzureSpeechData Parameters;
};


namespace AzureSpeech
{
	static void AsyncVoiceToText(const FAzureSpeechData Parameters, FVoiceToTextDelegate Delegate)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Parameters, Delegate]()
		{
			const TFuture<wchar_t*> VoiceToTextAsyncWork =
				Async(EAsyncExecution::Thread, [Parameters]() -> wchar_t*
				{
					const std::string SubscriptionID = std::string(TCHAR_TO_UTF8(*Parameters.SubscriptionID));
					const std::string RegionID = std::string(TCHAR_TO_UTF8(*Parameters.RegionID));
					const std::string LanguageID = std::string(TCHAR_TO_UTF8(*Parameters.LanguageID));

					return FAzureSpeechWrapper::DoVoiceToTextWork(SubscriptionID, RegionID, LanguageID);
				});

			VoiceToTextAsyncWork.WaitFor(FTimespan::FromSeconds(5));
			const FString& RecognizedString = VoiceToTextAsyncWork.Get();

			AsyncTask(ENamedThreads::GameThread, [RecognizedString, Delegate]()
			{
				Delegate.Broadcast(RecognizedString);
			});

			UE_LOG(LogTemp, Warning,
			       TEXT("AzureSpeech Debug - Subscription: %s, Region: %s, Language: %s, Voice To Text Result: %s"),
			       *FString(Parameters.SubscriptionID), *FString(Parameters.RegionID), *FString(Parameters.LanguageID),
			       *RecognizedString);
		});
	}
} // namespace AzureSpeech
