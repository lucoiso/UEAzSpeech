// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

using System;
using System.IO;
using UnrealBuildTool;

public class AzSpeech : ModuleRules
{
    public AzSpeech(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Engine",
                "Core",
                "CoreUObject",
                "Projects",
                "AzureWrapper"
            }
        );
    }
}