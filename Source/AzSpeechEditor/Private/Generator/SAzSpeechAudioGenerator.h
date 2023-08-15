// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Widgets/SCompoundWidget.h>

class SAzSpeechAudioGenerator final : public SCompoundWidget
{
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

    TSharedPtr<class STextComboBox> VoiceComboBox;
    TSharedPtr<class SEditableTextBox> PathInput;
    TSharedPtr<class SEditableTextBox> AssetNameInput;

    TSharedRef<SWidget> ConstructContent();

    TArray<TSharedPtr<FString>> GetStringArrayAsSharedPtr(const TArray<FString>& Input) const;
    TArray<TSharedPtr<FString>> GetAvailableContentModules() const;

    void OnAvailableVoicesChanged(const TArray<FString>& Voices);
    void OnFileInfoCommited(const FText& InText, FString& Member, TSharedPtr<SEditableTextBox>& InputRef);
};
