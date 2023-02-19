// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "SAzSpeechAudioGenerator.h"
#include <AzSpeechInternalFuncs.h>
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
#include <Algo/RemoveIf.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SAzSpeechAudioGenerator)
#endif

UAzSpeechPropertiesGetter::UAzSpeechPropertiesGetter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UAzSpeechPropertiesGetter::OnAvailableVoicesChanged(const TArray<FString>& Voices)
{
	if (OwningWidget)
	{
		OwningWidget->OnAvailableVoicesChanged(Voices);
	}

	Destroy();
}

void UAzSpeechPropertiesGetter::SynthesisCompleted(const TArray<uint8>& AudioData)
{
	if (OwningWidget)
	{
		if (USoundWave* const SoundWave = UAzSpeechHelper::ConvertAudioDataToSoundWave(AudioData, OwningWidget->Module, OwningWidget->RelativePath, OwningWidget->AssetName))
		{
			UGameplayStatics::PlaySound2D(GEditor->GetEditorWorldContext().World(), SoundWave);
		}
	}

	Destroy();
}

void UAzSpeechPropertiesGetter::TaskFail()
{
	Destroy();
}

void UAzSpeechPropertiesGetter::Destroy()
{
	OwningWidget = nullptr;

#if ENGINE_MAJOR_VERSION >= 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void SAzSpeechAudioGenerator::Construct([[maybe_unused]] const FArguments&)
{
	constexpr float Slot_Padding = 1.f;
	const ISlateStyle& AppStyle = FAppStyle::Get();

	const auto CenterTextCreator_Lambda = [&AppStyle](const FString& InStr) -> const TSharedRef<STextBlock>
	{
		return SNew(STextBlock)
			.Text(FText::FromString(InStr))
			.TextStyle(AppStyle, "PropertyEditor.AssetClass")
			.Font(AppStyle.GetFontStyle("PropertyWindow.NormalFont"))
			.Justification(ETextJustify::Left)
			.Margin(4.f);
	};

	const auto ContentPairCreator_Lambda = [this, &AppStyle](const TSharedRef<SWidget> Content1, const TSharedRef<SWidget> Content2) -> const TSharedRef<SBorder>
	{
		return SNew(SBorder)
			.BorderImage(AppStyle.GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.35f)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					Content1
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.65f)
				.MaxWidth(400.f)
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

	TArray<FModuleStatus> ModuleStatusArray;
	FModuleManager::Get().QueryModules(ModuleStatusArray);

	const int32 Index = Algo::RemoveIf(ModuleStatusArray, [](const FModuleStatus& ModuleStatus) {
		return ModuleStatus.bIsGameModule || !ModuleStatus.bIsLoaded || !FPaths::IsUnderDirectory(ModuleStatus.FilePath, FPaths::ProjectDir()) || AzSpeech::Internal::HasEmptyParam(ModuleStatus.Name);
	});

	ModuleStatusArray.RemoveAt(Index, ModuleStatusArray.Num() - Index);

	TArray<FString> ModuleNames;
	for (const FModuleStatus& ModuleStatus : ModuleStatusArray)
	{
		ModuleNames.Add(ModuleStatus.Name);
	}

	AvailableModules = GetStringArrayAsSharedPtr(ModuleNames);
	AvailableModules.Insert(MakeShareable(new FString("Game")), 0);

	VoiceComboBox = SNew(STextComboBox)
					.OptionsSource(&AvailableVoices)
					.InitiallySelectedItem(SelectVoice)
					.OnSelectionChanged_Lambda([this](const TSharedPtr<FString>& InStr, [[maybe_unused]] ESelectInfo::Type)
					{
						Voice = InStr->TrimStartAndEnd();
					});

	ChildSlot
	[
		SNew(SBorder)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding)
				.AutoHeight()
				[
					ContentPairCreator_Lambda(CenterTextCreator_Lambda("Synthesis Text/SSML"), SNew(SMultiLineEditableTextBox)
					.OnTextChanged(FOnTextChanged::CreateLambda([this](const FText& InText)
					{
						SynthesisText = InText;
					})))
				]
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding)
				.AutoHeight()
				[
					ContentPairCreator_Lambda(CenterTextCreator_Lambda("SSML"), SNew(SCheckBox)
					.IsChecked(ECheckBoxState::Unchecked)
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
					{
						bIsSSMLBased = InState == ECheckBoxState::Checked;
					}))
				]
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding)
				.AutoHeight()
				[
					ContentPairCreator_Lambda(CenterTextCreator_Lambda("Locale"), SNew(SEditableTextBox)
					.OnTextCommitted_Lambda([this](const FText& InText, [[maybe_unused]] ETextCommit::Type InCommitType)
					{
						Locale = InText.ToString().TrimStartAndEnd();
						VoiceComboBox->SetSelectedItem(SelectVoice);

						if (Locale.Equals("Auto", ESearchCase::IgnoreCase))
						{
							AvailableVoices = { SelectVoice };
						}
						else if (!InText.IsEmpty())
						{
							UpdateAvailableVoices();
						}
					}))
				]
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding)
				.AutoHeight()
				[
					ContentPairCreator_Lambda(CenterTextCreator_Lambda("Voice"), VoiceComboBox.ToSharedRef())
				]
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding)
				.AutoHeight()
				[
					ContentPairCreator_Lambda(CenterTextCreator_Lambda("Module"), SNew(STextComboBox)
					.OptionsSource(&AvailableModules)
					.OnSelectionChanged(STextComboBox::FOnTextSelectionChanged::CreateLambda([this](const TSharedPtr<FString>& InStr, [[maybe_unused]] ESelectInfo::Type)
					{
						Module = InStr->TrimStartAndEnd();
					})))
				]
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding)
				.AutoHeight()
				[
					ContentPairCreator_Lambda(CenterTextCreator_Lambda("Relative Path"), SNew(SEditableTextBox)
					.OnTextCommitted_Lambda([this](const FText& InText, [[maybe_unused]] ETextCommit::Type InCommitType)
					{
						RelativePath = InText.ToString().TrimStartAndEnd();
					}))
				]
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding)
				.AutoHeight()
				[
					ContentPairCreator_Lambda(CenterTextCreator_Lambda("Asset Name"), SNew(SEditableTextBox)
					.OnTextCommitted_Lambda([this](const FText& InText, [[maybe_unused]] ETextCommit::Type InCommitType)
					{
						AssetName = InText.ToString().TrimStartAndEnd();
					}))
				]
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding * 2.f)
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
	InternalGetter->OwningWidget = this;

	UGetAvailableVoicesAsync* const Task = UGetAvailableVoicesAsync::GetAvailableVoicesAsync(GEditor->GetEditorWorldContext().World(), Locale);
	Task->Success.AddDynamic(InternalGetter, &UAzSpeechPropertiesGetter::OnAvailableVoicesChanged);
	Task->Fail.AddDynamic(InternalGetter, &UAzSpeechPropertiesGetter::TaskFail);
	Task->Activate();
}

FReply SAzSpeechAudioGenerator::HandleGenerateAudioButtonClicked()
{
	UAzSpeechPropertiesGetter* const InternalGetter = NewObject<UAzSpeechPropertiesGetter>();
	InternalGetter->OwningWidget = this;

	UAzSpeechAudioDataSynthesisBase* Task = nullptr;
	if (bIsSSMLBased)
	{
		Task = USSMLToAudioDataAsync::SSMLToAudioData(GEditor->GetEditorWorldContext().World(), SynthesisText.ToString());
		Cast<USSMLToAudioDataAsync>(Task)->SynthesisCompleted.AddDynamic(InternalGetter, &UAzSpeechPropertiesGetter::SynthesisCompleted);
	}
	else
	{
		Task = UTextToAudioDataAsync::TextToAudioData(GEditor->GetEditorWorldContext().World(), SynthesisText.ToString(), Voice, Locale);
		Cast<UTextToAudioDataAsync>(Task)->SynthesisCompleted.AddDynamic(InternalGetter, &UAzSpeechPropertiesGetter::SynthesisCompleted);
	}

	Task->Activate();

	return FReply::Handled();
}

bool SAzSpeechAudioGenerator::IsGenerationEnabled() const
{
	if (bIsSSMLBased)
	{
		return !AzSpeech::Internal::HasEmptyParam(Module, AssetName, SynthesisText);
	}

	return !Voice.Equals(*SelectVoice.Get()) && !AzSpeech::Internal::HasEmptyParam(Voice, Module, AssetName, SynthesisText);
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