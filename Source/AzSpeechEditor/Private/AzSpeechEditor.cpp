// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeechEditor.h"
#include <ToolMenus.h>
#include <Widgets/Docking/SDockTab.h>

static const FName AzSpeechEditorTabName("AzSpeechEditor");

#define LOCTEXT_NAMESPACE "FAzSpeechEditorModule"

void FAzSpeechEditorModule::StartupModule()
{
	const auto RegisterDelegate = FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAzSpeechEditorModule::RegisterMenus);
	UToolMenus::RegisterStartupCallback(RegisterDelegate);
}

void FAzSpeechEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AzSpeechEditorTabName);
}

TSharedRef<SDockTab> FAzSpeechEditorModule::OnSpawnTab(const FSpawnTabArgs& SpawnTabArgs) const
{
	FText WidgetText = LOCTEXT("WindowWidgetText", "Work in Progress! :)");

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FAzSpeechEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	const auto EditorTabSpawnerDelegate = FOnSpawnTab::CreateRaw(this, &FAzSpeechEditorModule::OnSpawnTab);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AzSpeechEditorTabName, EditorTabSpawnerDelegate)
		.SetDisplayName(FText::FromString("AzSpeech Audio Generator"))
		.SetTooltipText(FText::FromString("Open Audio Generator Window"))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Package"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAzSpeechEditorModule, AzSpeechEditor)