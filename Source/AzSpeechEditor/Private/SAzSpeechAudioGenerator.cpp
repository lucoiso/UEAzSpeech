// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "SAzSpeechAudioGenerator.h"
#include <AzSpeechInternalFuncs.h>
#include <AzSpeech/AzSpeechSettings.h>
#include <AzSpeech/AzSpeechHelper.h>
#include <AzSpeech/Tasks/GetAvailableVoicesAsync.h>
#include <AzSpeech/Tasks/TextToAudioDataAsync.h>
#include <AzSpeech/Tasks/SSMLToAudioDataAsync.h>
#include <Widgets/Input/STextComboBox.h>
#include <Widgets/Input/SEditableTextBox.h>
#include <Widgets/Input/SMultiLineEditableTextBox.h>
#include <Widgets/Layout/SScrollBox.h>
#include <Kismet/GameplayStatics.h>
#include <Sound/SoundWave.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SAzSpeechAudioGenerator)
#endif

UAzSpeechPropertiesGetter::UAzSpeechPropertiesGetter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UAzSpeechPropertiesGetter::OnAvailableVoicesChanged(const TArray<FString>& Voices)
{
    OnAvailableVoicesUpdated.ExecuteIfBound(Voices);
    Destroy();
}

void UAzSpeechPropertiesGetter::SynthesisCompleted(const TArray<uint8>& AudioData)
{
    OnAudioDataGenerated.ExecuteIfBound(AudioData);
    Destroy();
}

void UAzSpeechPropertiesGetter::TaskFail()
{
    Destroy();
}

void UAzSpeechPropertiesGetter::Destroy()
{
    ClearFlags(RF_Standalone);

#if ENGINE_MAJOR_VERSION >= 5
    MarkAsGarbage();
#else
    MarkPendingKill();
#endif
}

void SAzSpeechAudioGenerator::Construct([[maybe_unused]] const FArguments&)
{
    constexpr float Slot_Padding = 1.f;
    constexpr float Margin_Spacing = Slot_Padding * 4.f;

#if ENGINE_MAJOR_VERSION < 5
    using FAppStyle = FEditorStyle;
#endif

    const ISlateStyle& AppStyle = FAppStyle::Get();

    const auto CenterTextCreator_Lambda = [&AppStyle, &Margin_Spacing](const FString& InStr) -> const TSharedRef<STextBlock>
    {
        return SNew(STextBlock)
            .Text(FText::FromString(InStr))
            .TextStyle(AppStyle, "PropertyEditor.AssetClass")
            .Font(AppStyle.GetFontStyle("PropertyWindow.NormalFont"))
            .Justification(ETextJustify::Left)
            .Margin(Margin_Spacing);
    };

    const auto ContentPairCreator_Lambda = [this, &AppStyle, &Slot_Padding](const TSharedRef<SWidget> Content1, const TSharedRef<SWidget> Content2) -> const TSharedRef<SBorder>
    {
        return SNew(SBorder)
            .BorderImage(AppStyle.GetBrush("ToolPanel.GroupBorder"))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(0.4f)
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Center)
                [
                    Content1
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.6f)
                .MaxWidth(400)
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Center)
                [
                    Content2
                ]
            ];
    };

    TextTypes = GetStringArrayAsSharedPtr({ "Text", "SSML" });
    SelectVoice = MakeShareable(new FString("Select a Voice (Set Locale to Update the List)"));

    AvailableVoices = { SelectVoice };

    GameModule = MakeShareable(new FString("Game"));
    Module = *GameModule.Get();

    AvailableModules = { GameModule };
    AvailableModules.Append(GetAvailableContentModules());

    ChildSlot
    [
        SNew(SBorder)
        .Padding(Margin_Spacing)
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .Padding(Slot_Padding)
                .AutoHeight()
                [
                    ContentPairCreator_Lambda(
                        CenterTextCreator_Lambda("Synthesis Text/SSML"),
                        SNew(SMultiLineEditableTextBox)
                        .OnTextChanged_Lambda(
                            [this](const FText& InText)
                            {
                                SynthesisText = InText;
                            }
                        )
                    )
                ]
                + SVerticalBox::Slot()
                .Padding(Slot_Padding)
                .AutoHeight()
                [
                    ContentPairCreator_Lambda(
                        CenterTextCreator_Lambda("SSML"),
                        SNew(SCheckBox)
                        .IsChecked(ECheckBoxState::Unchecked)
                        .OnCheckStateChanged_Lambda(
                            [this](const ECheckBoxState InState)
                            {
                                bIsSSMLBased = InState == ECheckBoxState::Checked;
                            }
                        )
                    )
                ]
                + SVerticalBox::Slot()
                .Padding(Slot_Padding)
                .AutoHeight()
                [
                    ContentPairCreator_Lambda(
                        CenterTextCreator_Lambda("Locale"), 
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
                    )
                ]
                + SVerticalBox::Slot()
                .Padding(Slot_Padding)
                .AutoHeight()
                [
                    ContentPairCreator_Lambda(
                        CenterTextCreator_Lambda("Voice"), 
                        SAssignNew(VoiceComboBox, STextComboBox)
                        .OptionsSource(&AvailableVoices)
                        .InitiallySelectedItem(SelectVoice)
                        .OnSelectionChanged_Lambda(
                            [this](const TSharedPtr<FString>& InStr, [[maybe_unused]] ESelectInfo::Type)
                            {
                                Voice = InStr->TrimStartAndEnd();
                            }
                        )
                    )
                ]
                + SVerticalBox::Slot()
                .Padding(Slot_Padding)
                .AutoHeight()
                [
                    ContentPairCreator_Lambda(
                        CenterTextCreator_Lambda("Module"),
                        SNew(STextComboBox)
                        .OptionsSource(&AvailableModules)
                        .InitiallySelectedItem(GameModule)
                        .OnSelectionChanged_Lambda(
                            [this](const TSharedPtr<FString>& InStr, [[maybe_unused]] ESelectInfo::Type)
                            {
                                Module = InStr->TrimStartAndEnd();
                            }
                        )
                    )
                ]
                + SVerticalBox::Slot()
                .Padding(Slot_Padding)
                .AutoHeight()
                [
                    ContentPairCreator_Lambda(
                        CenterTextCreator_Lambda("Relative Path"),
                        SAssignNew(PathInput, SEditableTextBox)
                        .OnTextCommitted_Lambda(
                            [this](const FText& InText, [[maybe_unused]] ETextCommit::Type InCommitType)
                            {
                                OnFileInfoCommited(InText, RelativePath, PathInput);
                            }
                        )
                    )
                ]
                + SVerticalBox::Slot()
                .Padding(Slot_Padding)
                .AutoHeight()
                [
                    ContentPairCreator_Lambda(
                        CenterTextCreator_Lambda("Asset Name"),
                        SAssignNew(AssetNameInput, SEditableTextBox)
                        .OnTextCommitted_Lambda(
                            [this](const FText& InText, [[maybe_unused]] ETextCommit::Type InCommitType)
                            {
                                OnFileInfoCommited(InText, AssetName, AssetNameInput);
                            }
                        )
                    )
                ]
                + SVerticalBox::Slot()
                .Padding(Margin_Spacing)
                .HAlign(HAlign_Center)
                .AutoHeight()
                [
                    SNew(SButton)
                    .Text(FText::FromString("Generate Audio"))
                    .OnClicked(this, &SAzSpeechAudioGenerator::HandleGenerateAudioButtonClicked)
                    .IsEnabled(this, &SAzSpeechAudioGenerator::IsGenerationEnabled)
                ]
            ]
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

void SAzSpeechAudioGenerator::OnAvailableVoicesChanged(const TArray<FString>& Voices)
{
    VoiceComboBox->SetSelectedItem(SelectVoice);
    AvailableVoices = { SelectVoice };
    AvailableVoices.Append(GetStringArrayAsSharedPtr(Voices));
}

TArray<TSharedPtr<FString>> SAzSpeechAudioGenerator::GetStringArrayAsSharedPtr(const TArray<FString>& Input) const
{
    TArray<TSharedPtr<FString>> Output;

    for (const auto& String : Input)
    {
        Output.Add(MakeShareable<FString>(new FString(String)));
    }

    return Output;
}

TArray<TSharedPtr<FString>> SAzSpeechAudioGenerator::GetAvailableContentModules() const
{
    TArray<FString> Output = UAzSpeechHelper::GetAvailableContentModules();
    Output.Remove("Game");
    return GetStringArrayAsSharedPtr(Output);
}

void SAzSpeechAudioGenerator::OnFileInfoCommited(const FText& InText, FString& Member, TSharedPtr<SEditableTextBox>& InputRef)
{
    const FString NewValue = FText::TrimPrecedingAndTrailing(InText).ToString();
    Member = FPaths::MakeValidFileName(NewValue);

    InputRef->SetText(FText::FromString(Member));
}