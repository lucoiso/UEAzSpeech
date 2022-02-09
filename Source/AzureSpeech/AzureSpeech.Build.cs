// Lucas Vilas-Boas - 2022

using System;
using System.IO;
using UnrealBuildTool;

public class AzureSpeech : ModuleRules
{
    public AzureSpeech(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new[]
            {
                IncludePath
            }
        );

        PrivateIncludePaths.AddRange(
            new[]
            {
                "AzureSpeech/Private"
            }
        );

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Engine",
                "Core",
                "CoreUObject",
                "Projects"
            }
        );

        Console.WriteLine(LoadSpeechLib(Target)
            ? "AzureSpeech loaded."
            : "AzureSpeech current only support Win64 builds.");
    }

    private string ThirdPartyPath
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/"));
        }
    }

    private string BinariesPath
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Binaries/"));
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

    public string GetUProjectPath()
    {
        return Path.Combine(ModuleDirectory, "../../../..");
    }

    private int HashFile(string FilePath)
    {
        var DLLString = File.ReadAllText(FilePath);
        return DLLString.GetHashCode() + DLLString.Length;
    }

    private void LoadAndCopyDLL(ReadOnlyTargetRules Target, string PlatformName, string DLLFileName)
    {
        var PluginLibDLLPath = Path.Combine(LibraryPath, DLLFileName);
        CopyToPluginBinaries(PluginLibDLLPath, Target);

        var PluginDLLPath = Path.Combine(BinariesPath, PlatformName, DLLFileName);
        Console.WriteLine("AzureSpeech: using dll at " + PluginDLLPath);
        CopyToProjectBinaries(PluginDLLPath, Target);

        var DLLPath = Path.GetFullPath(Path.Combine(GetUProjectPath(), "Binaries", PlatformName, DLLFileName));
        RuntimeDependencies.Add(DLLPath);
    }

    private void CopyToPluginBinaries(string Filepath, ReadOnlyTargetRules Target)
    {
        var BinariesDir = Path.Combine(BinariesPath, Target.Platform.ToString());
        var Filename = Path.GetFileName(Filepath);

        var FullBinariesDir = Path.GetFullPath(BinariesDir);

        if (!Directory.Exists(FullBinariesDir)) Directory.CreateDirectory(FullBinariesDir);

        var FullExistingPath = Path.Combine(FullBinariesDir, Filename);
        var ValidFile = false;

        if (File.Exists(FullExistingPath))
        {
            var ExistingFileHash = HashFile(FullExistingPath);
            var TargetFileHash = HashFile(Filepath);

            ValidFile = ExistingFileHash == TargetFileHash;
            if (!ValidFile) Console.WriteLine("AzureSpeech: outdated dll detected.");
        }

        if (ValidFile) return;

        Console.WriteLine(
            "AzureSpeech: Copied from " + Filepath + ", to " + Path.Combine(FullBinariesDir, Filename));
        File.Copy(Filepath, Path.Combine(FullBinariesDir, Filename), true);
    }

    private void CopyToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
    {
        var BinariesDir = Path.Combine(GetUProjectPath(), "Binaries", Target.Platform.ToString());
        var Filename = Path.GetFileName(Filepath);

        var FullBinariesDir = Path.GetFullPath(BinariesDir);

        if (!Directory.Exists(FullBinariesDir)) Directory.CreateDirectory(FullBinariesDir);

        var FullExistingPath = Path.Combine(FullBinariesDir, Filename);
        var ValidFile = false;

        if (File.Exists(FullExistingPath))
        {
            var ExistingFileHash = HashFile(FullExistingPath);
            var TargetFileHash = HashFile(Filepath);
            ValidFile = ExistingFileHash == TargetFileHash;
            if (!ValidFile) Console.WriteLine("AzureSpeech: outdated dll detected.");
        }

        if (ValidFile) return;

        Console.WriteLine(
            "AzureSpeech: Copied from " + Filepath + ", to " + Path.Combine(FullBinariesDir, Filename));
        File.Copy(Filepath, Path.Combine(FullBinariesDir, Filename), true);
    }

    public bool LoadSpeechLib(ReadOnlyTargetRules Target)
    {
        if (Target.Platform != UnrealTargetPlatform.Win64) return false;
        
        var PlatformString = Target.Platform.ToString();

        PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "Microsoft.CognitiveServices.Speech.core.lib"));

        LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.core.dll");
        LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.extension.audio.sys.dll");
        LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.extension.codec.dll");
        LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.extension.kws.dll");
        LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.extension.lu.dll");
        LoadAndCopyDLL(Target, PlatformString, "Microsoft.CognitiveServices.Speech.extension.silk_codec.dll");

        return true;
    }
}