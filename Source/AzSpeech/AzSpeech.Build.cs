// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

using System.IO;
using UnrealBuildTool;

public class AzSpeech : ModuleRules
{
    public AzSpeech(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;
        bEnableExceptions = true;

        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "AzureWrapper"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Engine",
            "CoreUObject",
            "Projects",
            "AndroidPermission",
            "DeveloperSettings",
            "AudioCaptureCore",
            "AssetRegistry",
            "Projects",
            "Json"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }

        PrivateIncludePathModuleNames.Add("DesktopPlatform");
    }
}