// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "AzSpeechRecognitionMap.generated.h"

USTRUCT(BlueprintType, Category = "AzSpeech")
struct FAzSpeechRecognitionData
{
	GENERATED_USTRUCT_BODY()
	
	FAzSpeechRecognitionData() = default;

	FAzSpeechRecognitionData(const int32 InValue) : Value(InValue) {};

	/* Value that will be returned if this recognition data matches the checked string */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech", Meta = (ClampMin = "0", UIMin = "0"))
	int32 Value = 0;

	/* Weight property to use in recognition checks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech", Meta = (ClampMin = "1", UIMin = "1"))
	int32 Weight = 1;

	/* Keys that will define if this recognition data is a good match */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	TArray<FString> TriggerKeys;

	/* If the recognized string contains any of this ignore keys, this recognition data will be ignored in the check */
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

	/* The name of this recognition data group */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	FName GroupName = NAME_None;

	/* Container of trigger/ignore keys and the values that they will returned if matches the recognized string */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech", Meta = (TitleProperty = "Value: {Value}"))
	TArray<FAzSpeechRecognitionData> RecognitionData;

	/* Requirement keys that the recognized string needs to contains at least 1 to check this group */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	TArray<FString> GlobalRequirementKeys;

	/* Ignore keys that will be applied to the entire group */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AzSpeech")
	TArray<FString> GlobalIgnoreKeys;

	bool operator==(const FAzSpeechRecognitionMap& Rhs) const
	{
		return GroupName == Rhs.GroupName || RecognitionData == Rhs.RecognitionData;
	}
};