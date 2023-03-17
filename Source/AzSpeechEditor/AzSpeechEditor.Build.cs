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

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "UnrealEd",
            "CoreUObject",
            "ToolMenus",
            "Engine",
            "EditorStyle",
            "ToolMenus",
            "Slate",
            "SlateCore",
            "WorkspaceMenuStructure",
            "AzSpeech"
        });
    }
}
