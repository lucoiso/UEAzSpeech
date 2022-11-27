// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/Tasks/Bases/AzSpeechTaskBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_speech_synthesis_result.h>
THIRD_PARTY_INCLUDES_END

#include "AzSpeechSynthesizerTaskBase.generated.h"

USTRUCT(BlueprintType, Category = "AzSpeech")
struct FAzSpeechVisemeData
{
	GENERATED_BODY()

	FAzSpeechVisemeData() = default;

	FAzSpeechVisemeData(const int32 InVisemeID, const int64 InAudioOffsetMilliseconds, const FString& InAnimation) : VisemeID(InVisemeID), AudioOffsetMilliseconds(InAudioOffsetMilliseconds), Animation(InAnimation)
	{
	}

	const bool IsValid() const
	{
		return VisemeID != -1 && AudioOffsetMilliseconds != -1;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	int32 VisemeID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	int64 AudioOffsetMilliseconds = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	FString Animation = FString();
};

class USoundWave;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVisemeReceived, const FAzSpeechVisemeData, VisemeData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudioDataSynthesisDelegate, const TArray<uint8>&, FinalAudioData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSoundWaveSynthesisDelegate, USoundWave*, GeneratedSound);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBooleanSynthesisDelegate, const bool, bSuccess);

/**
 *
 */
UCLASS(Abstract, MinimalAPI, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class UAzSpeechSynthesizerTaskBase : public UAzSpeechTaskBase
{
	GENERATED_BODY()

	friend class FAzSpeechSynthesisRunnable;

public:	
	virtual void Activate() override;

	/* Task delegate that will be called when dpdated */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FAzSpeechTaskGenericDelegate SynthesisUpdated;

	/* Task delegate that will be called when started */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FAzSpeechTaskGenericDelegate SynthesisStarted;

	/* Task delegate that will be called when receive a new viseme data */
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FVisemeReceived VisemeReceived;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FAzSpeechVisemeData GetLastVisemeData() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const TArray<FAzSpeechVisemeData> GetVisemeDataArray() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const TArray<uint8> GetLastSynthesizedAudioData() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const bool IsLastVisemeDataValid() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const bool IsLastResultValid() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FString GetVoiceName() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FString GetSynthesisText() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech", Meta = (DisplayName = "Is SSML Based"))
	const bool IsSSMLBased() const;
	
protected:
	FString VoiceName;
	FString SynthesisText;
	bool bIsSSMLBased;

	void StartSynthesisWork(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);
	
	const bool CanBroadcastWithReason(const Microsoft::CognitiveServices::Speech::ResultReason& Reason) const;
	
	virtual void OnVisemeReceived(const FAzSpeechVisemeData& VisemeData);
	virtual void OnSynthesisUpdate(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult);
	
private:
	std::vector<uint8_t> LastAudioBuffer;
	TArray<FAzSpeechVisemeData> VisemeDataArray;
	bool bLastResultIsValid = false;

	void LogSynthesisResultStatus(const bool bSuccess) const;
};
