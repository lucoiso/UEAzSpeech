// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>
#include "AzSpeech/Structures/AzSpeechRecognitionMap.h"
#include "AzSpeech/Structures/AzSpeechPhraseListMap.h"
#include "AzSpeech/Structures/AzSpeechSettingsOptions.h"
#include "AzSpeechSettings.generated.h"

/**
 *
 */
UCLASS(Config = Plugins, DefaultConfig, meta = (DisplayName = "AzSpeech"))
class AZSPEECH_API UAzSpeechSettings final : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    explicit UAzSpeechSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    static const UAzSpeechSettings* Get();

    UPROPERTY(GlobalConfig, EditAnywhere, Category = "AzSpeech", Meta = (DisplayName = "Default Options"))
    FAzSpeechSettingsOptions DefaultOptions;

    /* Time Out in seconds to wait for related asynchronous tasks to initialize: std::future::wait_for */
    UPROPERTY(GlobalConfig, EditAnywhere, Category = "Thread", Meta = (DisplayName = "Task Initialization Time Out in Seconds", ClampMin = "1", UIMin = "1", ClampMax = "600", UIMax = "600"))
    int32 TaskInitTimeOut;

    /* CPU thread priority to use in created runnable threads */
    UPROPERTY(GlobalConfig, EditAnywhere, Category = "Thread", Meta = (DisplayName = "Thread Priority"))
    EAzSpeechThreadPriority TasksThreadPriority;

    /* Thread update interval: Sleep time between task update checks */
    UPROPERTY(GlobalConfig, EditAnywhere, Category = "Thread", Meta = (DisplayName = "Thread Update Interval", ClampMin = "0.0001", UIMin = "0.0001", ClampMax = "1", UIMax = "1"))
    float ThreadUpdateInterval;

    /* If enabled, SSML synthesizers tasks with viseme output type set to FacialExpression will return only data that contains the Animation property */
    UPROPERTY(GlobalConfig, EditAnywhere, Category = "Information", Meta = (DisplayName = "Filter Viseme Facial Expression"))
    bool bFilterVisemeFacialExpression;

    /* If enabled, logs will be generated inside Saved/Logs/AzSpeech folder whenever a task fails - Disabled for Android, iOS & Shipping builds */
    UPROPERTY(GlobalConfig, EditAnywhere, Category = "Information", Meta = (DisplayName = "Enable Azure SDK Logs"))
    bool bEnableSDKLogs;

    /* Will print extra internal informations in log */
    UPROPERTY(GlobalConfig, EditAnywhere, Category = "Information", Meta = (DisplayName = "Enable Internal Logs"))
    bool bEnableInternalLogs;

    /* Will print extra debugging informations in log */
    UPROPERTY(GlobalConfig, EditAnywhere, Category = "Information", Meta = (DisplayName = "Enable Debugging Logs"))
    bool bEnableDebuggingLogs;

    /* Will print extra debugging informations in screen */
    UPROPERTY(GlobalConfig, EditAnywhere, Category = "Information", Meta = (DisplayName = "Enable Debugging Prints"))
    bool bEnableDebuggingPrints;

    /* Map of Phrase Lists used to improve recognition accuracy */
    UPROPERTY(GlobalConfig, EditAnywhere, Category = "Extras", Meta = (DisplayName = "Phrase List Map", TitleProperty = "Group: {GroupName}"))
    TArray<FAzSpeechPhraseListMap> PhraseListMap;

    /* String delimiters to use in recognition checks */
    UPROPERTY(GlobalConfig, EditAnywhere, Category = "Extras", Meta = (DisplayName = "String Delimiters"))
    FName StringDelimiters;

    /* Map of keywords to trigger or ignore in recognition interactions: Used by CheckReturnFromRecognitionMap task */
    UPROPERTY(GlobalConfig, EditAnywhere, Category = "Extras", Meta = (DisplayName = "Recognition Map", TitleProperty = "Group: {GroupName}"))
    TArray<FAzSpeechRecognitionMap> RecognitionMap;

    UFUNCTION(BlueprintPure, Category = "AzSpeech | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get Candidate Languages", CompactNodeTitle = "AzSpeech Candidate Languages"))
    static TArray<FName> GetCandidateLanguages();

    UFUNCTION(BlueprintPure, Category = "AzSpeech | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get Phrase List Map", CompactNodeTitle = "AzSpeech Phrase List Map"))
    static TArray<FAzSpeechPhraseListMap> GetPhraseListMap();

    UFUNCTION(BlueprintPure, Category = "AzSpeech | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get Recognition Map", CompactNodeTitle = "AzSpeech Recognition Map"))
    static TArray<FAzSpeechRecognitionMap> GetRecognitionMap();

    UFUNCTION(BlueprintPure, Category = "AzSpeech | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get String Delimiters", CompactNodeTitle = "AzSpeech String Delimiters"))
    static FName GetStringDelimiters();

    UFUNCTION(BlueprintPure, Category = "AzSpeech | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get Default Options", CompactNodeTitle = "AzSpeech Default Options"))
    static FAzSpeechSettingsOptions GetDefaultOptions();

    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Set Default Options"))
    static void SetDefaultOptions(const FAzSpeechSettingsOptions& Value);

protected:
#if WITH_EDITOR
    virtual void PreEditChange(FProperty* const PropertyAboutToChange) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    virtual void PostInitProperties() override;

    virtual void SetToDefaults();

    void SaveAndReload(const FName& PropertyName);

private:
    void ValidateCandidateLanguages(const bool bRemoveEmpties = false);
    void ToggleInternalLogs();
    void ValidateRecognitionMap();
    void ValidatePhraseList();
    void ValidateEndpoint();

public:
    static const bool CheckAzSpeechSettings();
    static const bool CheckAzSpeechSettings(const FAzSpeechSubscriptionOptions& Options);
};
