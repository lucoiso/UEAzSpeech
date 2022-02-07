// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class AzureSpeech : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ModulePath, "../ThirdParty/"));
        }
    }

    private string BinariesPath
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ModulePath, "../../Binaries/"));
        }
    }

    private string LibraryPath
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ThirdPartyPath, "lib"));
        }
    }

    private string IncludePath
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ThirdPartyPath, "include"));
        }
    }

    public AzureSpeech(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
                IncludePath,
            }
            );

        PrivateIncludePaths.AddRange(
            new string[] {
				// ... add other private include paths required here ...
                "AzureSpeech/Private",
                // IncludePath,
            }
            );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Engine",
                "Core",
                "CoreUObject",
                "Projects"
				// ... add other public dependencies that you statically link with here ...
			}
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
				// ... add private dependencies that you statically link with here ...
			}
            );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );

        if (LoadSpeechLib(Target))
            System.Console.WriteLine("AzureSpeech loaded.");
        else
            System.Console.WriteLine("AzureSpeech current only support Win64 builds.");
    }

    public string GetUProjectPath()
    {
        return Path.Combine(ModuleDirectory, "../../../..");
    }

    private int HashFile(string FilePath)
    {
        string DLLString = File.ReadAllText(FilePath);
        return DLLString.GetHashCode() + DLLString.Length;
    }

    private void LoadAndCopyDLL(ReadOnlyTargetRules Target, string PlatformName, string DLLFileName)
    {
        string PluginLibDLLPath = Path.Combine(LibraryPath, DLLFileName);
        CopyToPluginBinaries(PluginLibDLLPath, Target);

        string PluginDLLPath = Path.Combine(BinariesPath, PlatformName, DLLFileName);
        System.Console.WriteLine("Project plugin detected, using dll at " + PluginDLLPath);
        CopyToProjectBinaries(PluginDLLPath, Target);
        string DLLPath = Path.GetFullPath(Path.Combine(GetUProjectPath(), "Binaries", PlatformName, DLLFileName));
        RuntimeDependencies.Add(DLLPath);
    }

    private void CopyToPluginBinaries(string Filepath, ReadOnlyTargetRules Target)
    {
        string BinariesDir = Path.Combine(BinariesPath, Target.Platform.ToString());
        string Filename = Path.GetFileName(Filepath);

        string FullBinariesDir = Path.GetFullPath(BinariesDir);

        if (!Directory.Exists(FullBinariesDir))
        {
            Directory.CreateDirectory(FullBinariesDir);
        }

        string FullExistingPath = Path.Combine(FullBinariesDir, Filename);
        bool ValidFile = false;

        if (File.Exists(FullExistingPath))
        {
            int ExistingFileHash = HashFile(FullExistingPath);
            int TargetFileHash = HashFile(Filepath);
            ValidFile = ExistingFileHash == TargetFileHash;
            if (!ValidFile)
            {
                System.Console.WriteLine("AzureSpeech: outdated dll detected.");
            }
        }

        if (!ValidFile)
        {
            System.Console.WriteLine("AzureSpeech: Copied from " + Filepath + ", to " + Path.Combine(FullBinariesDir, Filename));
            File.Copy(Filepath, Path.Combine(FullBinariesDir, Filename), true);
        }
    }

    private void CopyToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
    {
        string BinariesDir = Path.Combine(GetUProjectPath(), "Binaries", Target.Platform.ToString());
        string Filename = Path.GetFileName(Filepath);

        string FullBinariesDir = Path.GetFullPath(BinariesDir);

        if (!Directory.Exists(FullBinariesDir))
        {
            Directory.CreateDirectory(FullBinariesDir);
        }

        string FullExistingPath = Path.Combine(FullBinariesDir, Filename);
        bool ValidFile = false;

        if (File.Exists(FullExistingPath))
        {
            int ExistingFileHash = HashFile(FullExistingPath);
            int TargetFileHash = HashFile(Filepath);
            ValidFile = ExistingFileHash == TargetFileHash;
            if (!ValidFile)
            {
                System.Console.WriteLine("AzureSpeech: outdated dll detected.");
            }
        }

        if (!ValidFile)
        {
            System.Console.WriteLine("AzureSpeech: Copied from " + Filepath + ", to " + Path.Combine(FullBinariesDir, Filename));
            File.Copy(Filepath, Path.Combine(FullBinariesDir, Filename), true);
        }
    }

    public bool LoadSpeechLib(ReadOnlyTargetRules Target)
    {
        bool IsLibrarySupported = false;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            IsLibrarySupported = true;

            string PlatformString = Target.Platform.ToString();

            PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "Microsoft.CognitiveServices.Speech.core.lib"));

            LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.core.dll");
            LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.extension.audio.sys.dll");
            LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.extension.codec.dll");
            LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.extension.kws.dll");
            LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.extension.lu.dll");
            LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.extension.silk_codec.dll");
        }

        return IsLibrarySupported;
    }
}