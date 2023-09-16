// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "SAzSpeechAudioGenerator.h"
#include "AzSpeechPropertiesGetter.h"
#include <AzSpeechInternalFuncs.h>
#include <AzSpeech/AzSpeechSettings.h>
#include <AzSpeech/AzSpeechHelper.h>
#include <AzSpeech/Tasks/Utils/GetAvailableVoicesAsync.h>
#include <AzSpeech/Tasks/Synthesis/TextToAudioDataAsync.h>
#include <AzSpeech/Tasks/Synthesis/SSMLToAudioDataAsync.h>
#include <Kismet/GameplayStatics.h>
#include <Sound/SoundWave.h>
#include <Widgets/Input/STextComboBox.h>
#include <Widgets/Input/SEditableTextBox.h>
#include <Widgets/Input/SMultiLineEditableTextBox.h>
#include <Widgets/Layout/SScrollBox.h>
#include <Widgets/Layout/SGridPanel.h>

void SAzSpeechAudioGenerator::Construct([[maybe_unused]] const FArguments&)
{
    TextTypes = GetStringArrayAsSharedPtr({ "Text", "SSML" });
    SelectVoice = MakeShared<FString>("Select a Voice (Set Locale to Update the List)");

    AvailableVoices = { SelectVoice };

    GameModule = MakeShared<FString>("Game");
    Module = *GameModule.Get();

    AvailableModules = { GameModule };
    AvailableModules.Append(GetAvailableContentModules());

    ChildSlot
        [
            ConstructContent()
        ];
}

TSharedRef<SWidget> SAzSpeechAudioGenerator::ConstructContent()
{
    constexpr float MarginSpacing = 4.f;

#if ENGINE_MAJOR_VERSION < 5
    using FAppStyle = FEditorStyle;
#endif

    return SNew(SScrollBox)
        + SScrollBox::Slot()
        .Padding(MarginSpacing)
        [
            SNew(SGridPanel)
                .FillColumn(1, 1.f)
                + SGridPanel::Slot(0, 0)
                .Padding(MarginSpacing)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Synthesis Text/SSML")))
                        .TextStyle(FAppStyle::Get(), "PropertyEditor.AssetClass")
                        .Font(FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont"))
                ]
                + SGridPanel::Slot(1, 0)
                .Padding(MarginSpacing)
                [
                    SNew(SMultiLineEditableTextBox)
                        .OnTextChanged_Lambda(
                            [this](const FText& InText)
                            {
                                SynthesisText = InText;
                            }
                        )
                ]
                + SGridPanel::Slot(0, 1)
                .Padding(MarginSpacing)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Is SSML")))
                        .TextStyle(FAppStyle::Get(), "PropertyEditor.AssetClass")
                        .Font(FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont"))
                ]
                + SGridPanel::Slot(1, 1)
                .Padding(MarginSpacing)
                [
                    SNew(SCheckBox)
                        .IsChecked(ECheckBoxState::Unchecked)
                        .OnCheckStateChanged_Lambda(
                            [this](const ECheckBoxState InState)
                            {
                                bIsSSMLBased = InState == ECheckBoxState::Checked;
                            }
                        )
                ]
                + SGridPanel::Slot(0, 2)
                .Padding(MarginSpacing)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Locale")))
                        .TextStyle(FAppStyle::Get(), "PropertyEditor.AssetClass")
                        .Font(FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont"))
                ]
                + SGridPanel::Slot(1, 2)
                .Padding(MarginSpacing)
                [
                    SNew(SEditableTextBox)
                        .OnTextCommitted_Lambda(
                            [this](const FText& InText, [[maybe_unused]] ETextCommit::Type InCommitType)
                            {
                                const FString NewValue = FText::TrimPrecedingAndTrailing(InText).ToString();

                                if (Locale.Equals(NewValue, ESearchCase::IgnoreCase))
                                {
                                    return;
                                }

                                Locale = NewValue;
                                VoiceComboBox->SetSelectedItem(SelectVoice);

                                if (Locale.Equals("Auto", ESearchCase::IgnoreCase))
                                {
                                    AvailableVoices = { SelectVoice };
                                }
                                else if (!AzSpeech::Internal::HasEmptyParam(Locale))
                                {
                                    UpdateAvailableVoices();
                                }
                            }
                        )
                ]
                + SGridPanel::Slot(0, 3)
                .Padding(MarginSpacing)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Voice")))
                        .TextStyle(FAppStyle::Get(), "PropertyEditor.AssetClass")
                        .Font(FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont"))
                ]
                + SGridPanel::Slot(1, 3)
                .Padding(MarginSpacing)
                [
                    SAssignNew(VoiceComboBox, STextComboBox)
                        .OptionsSource(&AvailableVoices)
                        .InitiallySelectedItem(SelectVoice)
                        .OnSelectionChanged_Lambda(
                            [this](const TSharedPtr<FString>& InStr, [[maybe_unused]] ESelectInfo::Type)
                            {
                                Voice = InStr->TrimStartAndEnd();
                            }
                        )
                ]
                + SGridPanel::Slot(0, 4)
                .Padding(MarginSpacing)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Module")))
                        .TextStyle(FAppStyle::Get(), "PropertyEditor.AssetClass")
                        .Font(FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont"))
                ]
                + SGridPanel::Slot(1, 4)
                .Padding(MarginSpacing)
                [
                    SNew(STextComboBox)
                        .OptionsSource(&AvailableModules)
                        .InitiallySelectedItem(GameModule)
                        .OnSelectionChanged_Lambda(
                            [this](const TSharedPtr<FString>& InStr, [[maybe_unused]] ESelectInfo::Type)
                            {
                                Module = InStr->TrimStartAndEnd();
                            }
                        )
                ]
                + SGridPanel::Slot(0, 5)
                .Padding(MarginSpacing)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Path (Relative to Module Root)")))
                        .TextStyle(FAppStyle::Get(), "PropertyEditor.AssetClass")
                        .Font(FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont"))
                ]
                + SGridPanel::Slot(1, 5)
                .Padding(MarginSpacing)
                [
                    SAssignNew(PathInput, SEditableTextBox)
                        .OnTextCommitted_Lambda(
                            [this](const FText& InText, [[maybe_unused]] ETextCommit::Type InCommitType)
                            {
                                OnFileInfoCommited(InText, RelativePath, PathInput);
                            }
                        )
                ]
                + SGridPanel::Slot(0, 6)
                .Padding(MarginSpacing)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Asset Name")))
                        .TextStyle(FAppStyle::Get(), "PropertyEditor.AssetClass")
                        .Font(FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont"))
                ]
                + SGridPanel::Slot(1, 6)
                .Padding(MarginSpacing)
                [
                    SAssignNew(AssetNameInput, SEditableTextBox)
                        .OnTextCommitted_Lambda(
                            [this](const FText& InText, [[maybe_unused]] ETextCommit::Type InCommitType)
                            {
                                OnFileInfoCommited(InText, AssetName, AssetNameInput);
                            }
                        )
                ]
                + SGridPanel::Slot(1, 7)
                .Padding(MarginSpacing)
                .HAlign(HAlign_Left)
                [
                    SNew(SButton)
                        .HAlign(HAlign_Center)
                        .Text(FText::FromString(TEXT("Generate Audio")))
                        .OnClicked(this, &SAzSpeechAudioGenerator::HandleGenerateAudioButtonClicked)
                        .IsEnabled(this, &SAzSpeechAudioGenerator::IsGenerationEnabled)
                ]
        ];
}

void SAzSpeechAudioGenerator::UpdateAvailableVoices()
{
    UAzSpeechPropertiesGetter* const InternalGetter = NewObject<UAzSpeechPropertiesGetter>();
    InternalGetter->SetFlags(RF_Standalone);

    InternalGetter->OnAvailableVoicesUpdated.BindLambda(
        [this](TArray<FString> Voices)
        {
            OnAvailableVoicesChanged(Voices);
        }
    );

    UGetAvailableVoicesAsync* const Task = UGetAvailableVoicesAsync::GetAvailableVoicesAsync(GEditor->GetEditorWorldContext().World(), Locale);
    Task->Success.AddDynamic(InternalGetter, &UAzSpeechPropertiesGetter::OnAvailableVoicesChanged);
    Task->Fail.AddDynamic(InternalGetter, &UAzSpeechPropertiesGetter::TaskFail);
    Task->Activate();
}

FReply SAzSpeechAudioGenerator::HandleGenerateAudioButtonClicked()
{
    if (!UAzSpeechSettings::CheckAzSpeechSettings())
    {
        FReply::Handled();
    }

    UAzSpeechPropertiesGetter* const InternalGetter = NewObject<UAzSpeechPropertiesGetter>();
    InternalGetter->SetFlags(RF_Standalone);

    InternalGetter->OnAudioDataGenerated.BindLambda(
        [this](TArray<uint8> AudioData)
        {
            if (USoundWave* const SoundWave = UAzSpeechHelper::ConvertAudioDataToSoundWave(AudioData, Module, RelativePath, AssetName))
            {
                UGameplayStatics::PlaySound2D(GEditor->GetEditorWorldContext().World(), SoundWave);
            }
        }
    );

    UAzSpeechAudioDataSynthesisBase* Task = nullptr;
    if (bIsSSMLBased)
    {
        Task = USSMLToAudioDataAsync::EditorTask(SynthesisText.ToString());
        Cast<USSMLToAudioDataAsync>(Task)->SynthesisCompleted.AddDynamic(InternalGetter, &UAzSpeechPropertiesGetter::SynthesisCompleted);
    }
    else
    {
        Task = UTextToAudioDataAsync::EditorTask(SynthesisText.ToString(), Voice, Locale);
        Cast<UTextToAudioDataAsync>(Task)->SynthesisCompleted.AddDynamic(InternalGetter, &UAzSpeechPropertiesGetter::SynthesisCompleted);
    }

    Task->Activate();

    return FReply::Handled();
}

bool SAzSpeechAudioGenerator::IsGenerationEnabled() const
{
    if (bIsSSMLBased)
    {
        return !AzSpeech::Internal::HasEmptyParam(Module, AssetName, SynthesisText.ToString());
    }

    return !Voice.Equals(*SelectVoice.Get()) && !AzSpeech::Internal::HasEmptyParam(Voice, Module, AssetName, SynthesisText.ToString());
}

TArray<TSharedPtr<FString>> SAzSpeechAudioGenerator::GetStringArrayAsSharedPtr(const TArray<FString>& Input) const
{
    TArray<TSharedPtr<FString>> Output;

    for (const auto& String : Input)
    {
        Output.Add(MakeShared<FString>(String));
    }

    return Output;
}

TArray<TSharedPtr<FString>> SAzSpeechAudioGenerator::GetAvailableContentModules() const
{
    TArray<FString> Output = UAzSpeechHelper::GetAvailableContentModules();
    Output.Remove("Game");

    return GetStringArrayAsSharedPtr(Output);
}

void SAzSpeechAudioGenerator::OnAvailableVoicesChanged(const TArray<FString>& Voices)
{
    VoiceComboBox->SetSelectedItem(SelectVoice);
    AvailableVoices = { SelectVoice };
    AvailableVoices.Append(GetStringArrayAsSharedPtr(Voices));
}

void SAzSpeechAudioGenerator::OnFileInfoCommited(const FText& InText, FString& Member, TSharedPtr<SEditableTextBox>& InputRef)
{
    const FString NewValue = FText::TrimPrecedingAndTrailing(InText).ToString();
    Member = FPaths::MakeValidFileName(NewValue);

    InputRef->SetText(FText::FromString(Member));
}