// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeechEditor.h"
#include "Generator/SAzSpeechAudioGenerator.h"
#include <ToolMenus.h>
#include <Widgets/Docking/SDockTab.h>
#include <WorkspaceMenuStructure.h>
#include <WorkspaceMenuStructureModule.h>

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
    return SNew(SDockTab)
        .TabRole(NomadTab)
        [
            SNew(SAzSpeechAudioGenerator)
        ];
}

void FAzSpeechEditorModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);
    const auto EditorTabSpawnerDelegate = FOnSpawnTab::CreateRaw(this, &FAzSpeechEditorModule::OnSpawnTab);

#if ENGINE_MAJOR_VERSION < 5
    const FName AppStyleName = FEditorStyle::GetStyleSetName();
#else
    const FName AppStyleName = FAppStyle::GetAppStyleSetName();
#endif

    const TSharedPtr<FWorkspaceItem> Menu = WorkspaceMenu::GetMenuStructure().GetToolsCategory()->AddGroup(LOCTEXT("AzSpeechCategory", "AzSpeech"), LOCTEXT("AzSpeechCategoryTooltip", "AzSpeech Plugin Tabs"), FSlateIcon(AppStyleName, "Icons.Package"));

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AzSpeechEditorTabName, EditorTabSpawnerDelegate)
        .SetDisplayName(FText::FromString(TEXT("AzSpeech Audio Generator")))
        .SetTooltipText(FText::FromString(TEXT("Open AzSpeech Audio Generator")))
        .SetIcon(FSlateIcon(AppStyleName, "Icons.Plus"))
        .SetGroup(Menu.ToSharedRef());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAzSpeechEditorModule, AzSpeechEditor)