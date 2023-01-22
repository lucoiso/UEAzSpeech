// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

using System;
using System.IO;
using UnrealBuildTool;

public class AzureWrapper : ModuleRules
{
	private bool isArm()
	{
		return Target.Architecture.ToLower().Contains("arm");
	}

	public AzureWrapper(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		bEnableExceptions = true;

		PublicIncludePaths.AddRange(new[]
		{
			Path.Combine(ModuleDirectory, "include", "c_api"),
			Path.Combine(ModuleDirectory, "include", "cxx_api")
		});

        if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "Win", "Microsoft.CognitiveServices.Speech.core.lib"));

			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.core.dll");
			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.extension.audio.sys.dll");
			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.extension.kws.dll");
			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.extension.lu.dll");
			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.extension.mas.dll");
			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.extension.codec.dll");

			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Win", "Runtime", "Microsoft.CognitiveServices.Speech.core.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Win", "Runtime", "Microsoft.CognitiveServices.Speech.extension.audio.sys.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Win", "Runtime", "Microsoft.CognitiveServices.Speech.extension.kws.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Win", "Runtime", "Microsoft.CognitiveServices.Speech.extension.lu.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Win", "Runtime", "Microsoft.CognitiveServices.Speech.extension.mas.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Win", "Runtime", "Microsoft.CognitiveServices.Speech.extension.codec.dll"));
		}
		else if (Target.Platform == UnrealTargetPlatform.HoloLens)
		{
            string libPath = isArm() ? "Arm64" : "x64";

            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "HoloLens", libPath, "Microsoft.CognitiveServices.Speech.core.lib"));
			
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "HoloLens", libPath, "Runtimes", "Microsoft.CognitiveServices.Speech.core.dll"));
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "HoloLens", libPath, "Runtimes", "Microsoft.CognitiveServices.Speech.extension.audio.sys.dll"));
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "HoloLens", libPath, "Runtimes", "Microsoft.CognitiveServices.Speech.extension.kws.dll"));
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "HoloLens", libPath, "Runtimes", "Microsoft.CognitiveServices.Speech.extension.lu.dll"));
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "HoloLens", libPath, "Runtimes", "Microsoft.CognitiveServices.Speech.extension.mas.dll"));

			if (!isArm())
            {
                RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "HoloLens", libPath, "Runtimes", "Microsoft.CognitiveServices.Speech.extension.codec.dll"));
            }
        }
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "AzSpeech_UPL_Android.xml"));

			// Linking both architectures: For some reason (or bug?) Target.Architecture is always empty ._.

			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "Android", "arm64-v8a", "libMicrosoft.CognitiveServices.Speech.core.so"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "Android", "arm64-v8a", "libMicrosoft.CognitiveServices.Speech.extension.audio.sys.so"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "Android", "arm64-v8a", "libMicrosoft.CognitiveServices.Speech.extension.kws.so"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "Android", "arm64-v8a", "libMicrosoft.CognitiveServices.Speech.extension.lu.so"));

			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "Android", "armeabi-v7a", "libMicrosoft.CognitiveServices.Speech.core.so"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "Android", "armeabi-v7a", "libMicrosoft.CognitiveServices.Speech.extension.audio.sys.so"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "Android", "armeabi-v7a", "libMicrosoft.CognitiveServices.Speech.extension.kws.so"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "Android", "armeabi-v7a", "libMicrosoft.CognitiveServices.Speech.extension.lu.so"));
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(ModuleDirectory, "AzSpeech_UPL_IOS.xml"));

			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "iOS", "libMicrosoft.CognitiveServices.Speech.core.a"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			string libPath = isArm() ? "Arm64" : "x64";
			string fullLibPath = Path.Combine(ModuleDirectory, "libs", "Mac", libPath, "libMicrosoft.CognitiveServices.Speech.core.dylib");

			PublicAdditionalLibraries.Add(fullLibPath);
			RuntimeDependencies.Add(fullLibPath);
		}
		else if (Target.Platform.ToString().ToLower().Contains("linux"))
		{
			string libPath = isArm() ? "Arm64" : "x64";

			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs", "Linux", libPath, "Microsoft.CognitiveServices.Speech.core.lib"));

			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Linux", libPath, "Runtime", "libMicrosoft.CognitiveServices.Speech.core.so"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Linux", libPath, "Runtime", "libMicrosoft.CognitiveServices.Speech.extension.audio.sys.so"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Linux", libPath, "Runtime", "libMicrosoft.CognitiveServices.Speech.extension.codec.so"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Linux", libPath, "Runtime", "libMicrosoft.CognitiveServices.Speech.extension.kws.so"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Linux", libPath, "Runtime", "libMicrosoft.CognitiveServices.Speech.extension.lu.so"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "libs", "Linux", libPath, "Runtime", "libMicrosoft.CognitiveServices.Speech.extension.mas.so"));
		}
	}
}