// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

using System;
using System.IO;
using System.Collections.Generic;
using UnrealBuildTool;

public class AzureWrapper : ModuleRules
{
    private bool isArmArch()
    {
        return Target.Architecture.ToString().ToLower().Contains("arm") || Target.Architecture.ToString().ToLower().Contains("aarch");
    }

    private string GetPlatformLibsSubDirectory()
    {
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            return Path.Combine("libs", "Win");
        }

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            return Path.Combine("libs", "Android");
        }

        if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            return Path.Combine("libs", "iOS");
        }

        string ArchSubDir = isArmArch() ? "Arm64" : "x64";

        if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            return Path.Combine("libs", "Mac", ArchSubDir);
        }

        if (Target.Platform.ToString().ToLower().Contains("linux"))
        {
            return Path.Combine("libs", "Linux", ArchSubDir);
        }

        return "UNDEFINED_DIRECTORY";
    }

    private string GetPlatformLibsAbsoluteDirectory()
    {
        return Path.Combine(ModuleDirectory, GetPlatformLibsSubDirectory());
    }

    private string GetRuntimesSubDirectory()
    {
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            return Path.Combine(GetPlatformLibsSubDirectory(), "Runtime");
        }

        return GetPlatformLibsSubDirectory();
    }

    private string GetRuntimesAbsoluteDirectory()
    {
        return Path.Combine(ModuleDirectory, GetRuntimesSubDirectory());
    }

    private List<string> GetStaticLibraries()
    {
        List<string> Output = new List<string>();

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            Output.AddRange(new[]
            {
                "Microsoft.CognitiveServices.Speech.core.lib"
            });
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            Output.AddRange(new[]
            {
				// arm64-v8a
				Path.Combine("arm64-v8a", "libMicrosoft.CognitiveServices.Speech.core.so"),
                Path.Combine("arm64-v8a", "libMicrosoft.CognitiveServices.Speech.extension.audio.sys.so"),
                Path.Combine("arm64-v8a", "libMicrosoft.CognitiveServices.Speech.extension.kws.so"),
                Path.Combine("arm64-v8a", "libMicrosoft.CognitiveServices.Speech.extension.kws.ort.so"),
                Path.Combine("arm64-v8a", "libMicrosoft.CognitiveServices.Speech.extension.lu.so"),

				// armeabi-v7a
				Path.Combine("armeabi-v7a", "libMicrosoft.CognitiveServices.Speech.core.so"),
                Path.Combine("armeabi-v7a", "libMicrosoft.CognitiveServices.Speech.extension.audio.sys.so"),
                Path.Combine("armeabi-v7a", "libMicrosoft.CognitiveServices.Speech.extension.kws.so"),
                Path.Combine("armeabi-v7a", "libMicrosoft.CognitiveServices.Speech.extension.kws.ort.so"),
                Path.Combine("armeabi-v7a", "libMicrosoft.CognitiveServices.Speech.extension.lu.so"),

				// x86_64
				Path.Combine("x86_64", "libMicrosoft.CognitiveServices.Speech.core.so"),
                Path.Combine("x86_64", "libMicrosoft.CognitiveServices.Speech.extension.audio.sys.so"),
                Path.Combine("x86_64", "libMicrosoft.CognitiveServices.Speech.extension.kws.so"),
                Path.Combine("x86_64", "libMicrosoft.CognitiveServices.Speech.extension.kws.ort.so"),
                Path.Combine("x86_64", "libMicrosoft.CognitiveServices.Speech.extension.lu.so")
            });
        }
        else if (Target.Platform.ToString().ToLower().Contains("linux"))
        {
            Output.AddRange(new[]
            {
                "libMicrosoft.CognitiveServices.Speech.core.so",
                "libMicrosoft.CognitiveServices.Speech.extension.audio.sys.so",
                "libMicrosoft.CognitiveServices.Speech.extension.kws.so",
                "libMicrosoft.CognitiveServices.Speech.extension.kws.ort.so",
                "libMicrosoft.CognitiveServices.Speech.extension.lu.so",
                "libMicrosoft.CognitiveServices.Speech.extension.codec.so"
            });
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            Output.AddRange(new[]
            {
                "libMicrosoft.CognitiveServices.Speech.core.a"
            });
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            Output.AddRange(new[]
            {
                "libMicrosoft.CognitiveServices.Speech.core.dylib"
            });
        }

        return Output;
    }

    private List<string> GetDynamicLibraries()
    {
        List<string> Output = new List<string>();

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            Output.AddRange(new[]
            {
                "Microsoft.CognitiveServices.Speech.core.dll",
                "Microsoft.CognitiveServices.Speech.extension.audio.sys.dll",
                "Microsoft.CognitiveServices.Speech.extension.kws.dll",
                "Microsoft.CognitiveServices.Speech.extension.kws.ort.dll",
                "Microsoft.CognitiveServices.Speech.extension.lu.dll",
                "Microsoft.CognitiveServices.Speech.extension.codec.dll"
            });
        }
        else if (Target.Platform.ToString().ToLower().Contains("linux") || Target.Platform == UnrealTargetPlatform.Mac)
        {
            // Using the same list as for static libraries
            Output = GetStaticLibraries();
        }
        else if (Target.Platform == UnrealTargetPlatform.Android || Target.Platform == UnrealTargetPlatform.IOS)
        {
            // Empty
        }

        return Output;
    }

    private bool IsRuntimePlatform()
    {
        return Target.Platform == UnrealTargetPlatform.Win64 ||
            Target.Platform == UnrealTargetPlatform.Mac ||
            Target.Platform.ToString().ToLower().Contains("linux");
    }

    private void InitializeRuntimeDefinitions()
    {
        if (!IsRuntimePlatform() || GetDynamicLibraries().Count <= 0)
        {
            return;
        }

        PublicDefinitions.Add(string.Format("AZSPEECH_WHITELISTED_BINARIES=\"{0}\"", string.Join(";", GetDynamicLibraries())));

        if (Target.Type != TargetType.Editor)
        {
            return;
        }

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add(string.Format("AZSPEECH_THIRDPARTY_BINARY_SUBDIR=\"{0}\"", GetRuntimesSubDirectory().Replace(@"\", @"\\")));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac || Target.Platform.ToString().ToLower().Contains("linux"))
        {
            PublicDefinitions.Add(string.Format("AZSPEECH_THIRDPARTY_BINARY_SUBDIR=\"{0}\"", GetRuntimesSubDirectory().Replace(@"\", @"/")));
        }
    }

    public AzureWrapper(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;
        Type = ModuleType.External;
        bEnableExceptions = true;

        PublicIncludePaths.AddRange(new[]
        {
            Path.Combine(ModuleDirectory, "include", "c_api"),
            Path.Combine(ModuleDirectory, "include", "cxx_api")
        });

        InitializeRuntimeDefinitions();

        foreach (string StaticLib in GetStaticLibraries())
        {
            PublicAdditionalLibraries.Add(Path.Combine(GetPlatformLibsAbsoluteDirectory(), StaticLib));
        }

        foreach (string DynamicLib in GetDynamicLibraries())
        {
            PublicDelayLoadDLLs.Add(DynamicLib);
            RuntimeDependencies.Add(Path.Combine(@"$(BinaryOutputDir)", DynamicLib), Path.Combine(GetRuntimesAbsoluteDirectory(), DynamicLib));
        }

        if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            // Experimental UPL usage for MacOS to add the required PList data
            AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(ModuleDirectory, "AzSpeech_UPL_MacOS.xml"));
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(ModuleDirectory, "AzSpeech_UPL_IOS.xml"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "AzSpeech_UPL_Android.xml"));
        }
    }
}