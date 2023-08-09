// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include "SAzSpeechAudioGenerator.generated.h"

DECLARE_DELEGATE_OneParam(FAvailableVoicesUpdated, TArray<FString>);
DECLARE_DELEGATE_OneParam(FAudioDataGenerated, TArray<uint8>);

UCLASS(MinimalAPI, NotBlueprintable, NotPlaceable, Category = "Implementation")
class UAzSpeechPropertiesGetter : public UObject
{
    GENERATED_BODY()

public:
    explicit UAzSpeechPropertiesGetter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    FAvailableVoicesUpdated OnAvailableVoicesUpdated;
    FAudioDataGenerated OnAudioDataGenerated;

    UFUNCTION()
    void OnAvailableVoicesChanged(const TArray<FString>& Voices);

    UFUNCTION()
    void SynthesisCompleted(const TArray<uint8>& AudioData);

    UFUNCTION()
    void TaskFail();

    virtual void Destroy();
};

class SAzSpeechAudioGenerator final : public SCompoundWidget
{
    friend class UAzSpeechPropertiesGetter;

public:
    SLATE_USER_ARGS(SAzSpeechAudioGenerator)
        {
        }

    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void UpdateAvailableVoices();

    FReply HandleGenerateAudioButtonClicked();
    bool IsGenerationEnabled() const;

private:
    bool bIsSSMLBased;
    FString Locale;
    FString Voice;
    FString Module;
    FString RelativePath;
    FString AssetName;
    FText SynthesisText;

    TSharedPtr<FString> SelectVoice;
    TSharedPtr<FString> GameModule;

    TArray<TSharedPtr<FString>> AvailableVoices;
    TArray<TSharedPtr<FString>> TextTypes;
    TArray<TSharedPtr<FString>> AvailableModules;

    void OnAvailableVoicesChanged(const TArray<FString>& Voices);
    TArray<TSharedPtr<FString>> GetStringArrayAsSharedPtr(const TArray<FString>& Input) const;
    TArray<TSharedPtr<FString>> GetAvailableContentModules() const;

    void OnFileInfoCommited(const FText& InText, FString& Member, TSharedPtr<SEditableTextBox>& InputRef);

    TSharedPtr<STextComboBox> VoiceComboBox;
    TSharedPtr<SEditableTextBox> PathInput;
    TSharedPtr<SEditableTextBox> AssetNameInput;
};
