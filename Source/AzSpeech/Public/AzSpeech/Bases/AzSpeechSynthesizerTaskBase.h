// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeech/Bases/AzSpeechTaskBase.h"

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_speech_synthesizer.h>
THIRD_PARTY_INCLUDES_END

#include "AzSpeechSynthesizerTaskBase.generated.h"

USTRUCT(BlueprintType, Category = "AzSpeech")
struct FAzSpeechVisemeData
{
	GENERATED_BODY()

	FAzSpeechVisemeData() = default;

	FAzSpeechVisemeData(const int32& InVisemeID, const int64& InAudioOffsetMilliseconds, const FString& InAnimation) : VisemeID(InVisemeID), AudioOffsetMilliseconds(InAudioOffsetMilliseconds), Animation(InAnimation)
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSynthesisUpdatedGenericDelegate);

/**
 *
 */
UCLASS(Abstract, MinimalAPI, NotPlaceable, Category = "AzSpeech", meta = (ExposedAsyncProxy = AsyncTask))
class UAzSpeechSynthesizerTaskBase : public UAzSpeechTaskBase
{
	GENERATED_BODY()

public:	
	virtual void Activate() override;
	virtual void StopAzSpeechTask() override;

	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FSynthesisUpdatedGenericDelegate SynthesisUpdated;

	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FVisemeReceived VisemeReceived;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FAzSpeechVisemeData GetLastVisemeData() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const TArray<uint8> GetLastSynthesizedStream() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const bool IsLastVisemeDataValid() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const bool IsLastResultValid() const;
	
protected:
	FString VoiceName;
	FString SynthesisText;

	bool bIsSSMLBased;
	bool bNullifySynthesizerObjectOnStop = false;

	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesizer> SynthesizerObject;

	virtual bool StartAzureTaskWork_Internal() override;
	virtual void ClearBindings() override;
	
	void EnableVisemeOutput();
	virtual void ApplyExtraSettings() override;

	virtual void ApplySDKSettings(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>& InConfig) override;

	virtual void OnVisemeReceived(const Microsoft::CognitiveServices::Speech::SpeechSynthesisVisemeEventArgs& VisemeEventArgs);
	virtual void OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs);

	bool InitializeSynthesizer(const std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig>& InAudioConfig);
	void StartSynthesisWork();

	void OutputSynthesisResult(const bool bSuccess) const;

	const bool ProcessSynthesisResult(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& Result) const;

	const bool CanBroadcastWithReason(const Microsoft::CognitiveServices::Speech::ResultReason& Reason) const;

private:
	FAzSpeechVisemeData LastVisemeData;
	std::vector<uint8_t> LastSynthesizedBuffer;
	bool bLastResultIsValid = false;
};
