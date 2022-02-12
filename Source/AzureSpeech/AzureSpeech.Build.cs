// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/AzureSpeech

using System;
using System.IO;
using UnrealBuildTool;

public class AzureSpeech : ModuleRules
{
    public AzureSpeech(ReadOnlyTargetRules Target) : base(Target)
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