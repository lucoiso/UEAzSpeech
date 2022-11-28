// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "AzSpeechRecognitionMap.generated.h"

USTRUCT(BlueprintType, Category = "AzSpeech")
struct FAzSpeechRecognitionData
{
	GENERATED_USTRUCT_BODY()
	
	FAzSpeechRecognitionData() = default;

	FAzSpeechRecognitionData(const int32 InValue) : Value(InValue) {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech", Meta = (ClampMin = "0", UIMin = "0"))
	int32 Value = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	TArray<FString> TriggerKeys;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	TArray<FString> IgnoreKeys;

	bool operator==(const FAzSpeechRecognitionData& Rhs) const
	{
		return Value == Rhs.Value || TriggerKeys == Rhs.TriggerKeys;
	}
};

USTRUCT(BlueprintType, Category = "AzSpeech")
struct FAzSpeechRecognitionMap
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	FName GroupName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech", Meta = (TitleProperty = "Value: {Value}"))
	TArray<FAzSpeechRecognitionData> RecognitionData;

	bool operator==(const FAzSpeechRecognitionMap& Rhs) const
	{
		return GroupName == Rhs.GroupName || RecognitionData == Rhs.RecognitionData;
	}
};