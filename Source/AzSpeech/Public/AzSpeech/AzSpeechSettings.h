// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>
#include <map>
#include <string>
#include "AzSpeech/Structures/AzSpeechRecognitionMap.h"
#include "AzSpeech/Structures/AzSpeechPhraseListMap.h"
#include "AzSpeech/Structures/AzSpeechSettingsOptions.h"
#include "AzSpeechSettings.generated.h"

constexpr int AZSPEECH_KEY_SUBSCRIPTION = 0;
constexpr int AZSPEECH_KEY_REGION = 1;
constexpr int AZSPEECH_KEY_ENDPOINT = 2;
constexpr int AZSPEECH_KEY_LANGUAGE = 3;
constexpr int AZSPEECH_KEY_VOICE = 4;

/**
 * 
 */
UCLASS(Config = Plugins, DefaultConfig, meta = (DisplayName="AzSpeech"))
class AZSPEECH_API UAzSpeechSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	explicit UAzSpeechSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static const UAzSpeechSettings* Get();

	static constexpr unsigned MaxCandidateLanguages = 10u;

	UPROPERTY(GlobalConfig, EditAnywhere, Meta = (DisplayName = "AzSpeech Default Settings"))
	FAzSpeechSettingsOptions Options;

	/* If enabled, logs will be generated inside Saved/Logs/AzSpeech folder whenever a task fails - Disabled for Android & Shipping builds */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Logging", Meta = (DisplayName = "Enable Azure SDK Logs"))
	bool bEnableSDKLogs;

	/* Will print extra internal informations in log */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Logging", Meta = (DisplayName = "Enable Internal Logs"))
	bool bEnableInternalLogs;

	/* Will print extra debugging informations in log */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Logging", Meta = (DisplayName = "Enable Debugging Logs"))
	bool bEnableDebuggingLogs;

	/* Map of Phrase Lists used to improve recognition accuracy */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Extras", Meta = (DisplayName = "Phrase List Map", TitleProperty = "Group: {GroupName}"))
	TArray<FAzSpeechPhraseListMap> PhraseListMap;

	/* String delimiters to use in recognition checks */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Extras", Meta = (DisplayName = "String Delimiters"))
	FString StringDelimiters;

	/* Map of keywords to trigger or ignore in recognition interactions: Used by CheckReturnFromRecognitionMap task */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Extras", Meta = (DisplayName = "Recognition Map", TitleProperty = "Group: {GroupName}"))
	TArray<FAzSpeechRecognitionMap> RecognitionMap;

	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get AzSpeech Settings Data: Candidate Languages"))
	static TArray<FString> GetCandidateLanguages();

	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get AzSpeech Settings Data: Phrase List Map"))
	static TArray<FAzSpeechPhraseListMap> GetPhraseListMap();

	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get AzSpeech Settings Data: Recognition Map"))
	static TArray<FAzSpeechRecognitionMap> GetRecognitionMap();

	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get AzSpeech Settings Data: String Delimiters"))
	static FString GetStringDelimiters();

	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get AzSpeech Settings Data: Default Settings"))
	static FAzSpeechSettingsOptions GetDefaultSettings();

	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Set AzSpeech Settings Data: Default Settings"))
	static void SetDefaultSettings(const FAzSpeechSettingsOptions& Value);

protected:
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostInitProperties() override;

private:
	void ValidateCandidateLanguages(const bool bRemoveEmpties = false);
	void ToggleInternalLogs();
	void ValidateRecognitionMap();
	void ValidatePhraseList();

public:
	static const std::map<int, std::string> GetAzSpeechKeys();
	static const bool CheckAzSpeechSettings();
};
