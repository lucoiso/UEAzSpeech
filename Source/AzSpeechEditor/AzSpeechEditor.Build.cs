// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

using System.IO;
using UnrealBuildTool;

public class AzSpeechEditor : ModuleRules
{
    public AzSpeechEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "AzSpeech"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Engine",
            "CoreUObject",
            "Slate",
            "SlateCore",
            "UnrealEd",
            "ToolMenus",
            "EditorStyle",
            "WorkspaceMenuStructure"
        });
    }
}