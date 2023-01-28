// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

using System;
using System.IO;
using System.Collections.Generic;
using UnrealBuildTool;

public class AzureWrapper : ModuleRules
{
	private bool isArmArch()
	{
		return Target.Architecture.ToLower().Contains("arm") || Target.Architecture.ToLower().Contains("aarch");
	}

	private string GetPlatformLibsDirectory()
	{
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			return Path.Combine(ModuleDirectory, "libs", "Win");
		}

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			return Path.Combine(ModuleDirectory, "libs", "Android");
		}

		string libPath = isArmArch() ? "Arm64" : "x64";

		if (Target.Platform == UnrealTargetPlatform.HoloLens)
		{
			return Path.Combine(ModuleDirectory, "libs", "HoloLens", libPath);
		}

		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			return Path.Combine(ModuleDirectory, "libs", "iOS", libPath);
		}

		if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			return Path.Combine(ModuleDirectory, "libs", "Mac", libPath);
		}

		if (Target.Platform.ToString().ToLower().Contains("linux"))
		{
			return Path.Combine(ModuleDirectory, "libs", "Linux", libPath);
		}

		return "UNDEFINED_DIRECTORY";
	}

	private List<string> GetLibsList()
	{
		List<string> Output = new List<string>();

		if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.HoloLens)
		{
			Output.AddRange(new[]
			{
				"Microsoft.CognitiveServices.Speech.core.dll",
				"Microsoft.CognitiveServices.Speech.extension.audio.sys.dll",
				"Microsoft.CognitiveServices.Speech.extension.kws.dll",
				"Microsoft.CognitiveServices.Speech.extension.lu.dll",
				"Microsoft.CognitiveServices.Speech.extension.mas.dll"
			});

			if (!isArmArch())
			{
				Output.Add("Microsoft.CognitiveServices.Speech.extension.codec.dll");
			}
		}

		else if (Target.Platform == UnrealTargetPlatform.Android || Target.Platform.ToString().ToLower().Contains("linux"))
		{
			Output.AddRange(new[]
			{
				"libMicrosoft.CognitiveServices.Speech.core.so",
				"libMicrosoft.CognitiveServices.Speech.extension.audio.sys.so",
				"libMicrosoft.CognitiveServices.Speech.extension.kws.so",
				"libMicrosoft.CognitiveServices.Speech.extension.lu.so"
			});

			if (Target.Platform.ToString().ToLower().Contains("linux"))
			{
				Output.AddRange(new[]
				{
					"libMicrosoft.CognitiveServices.Speech.extension.codec.so",
					"libMicrosoft.CognitiveServices.Speech.extension.mas.so"
				});
			}
		}

		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			Output.Add("libMicrosoft.CognitiveServices.Speech.core.a");
		}

		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			Output.Add("libMicrosoft.CognitiveServices.Speech.core.dylib");
		}

		return Output;
	}

	private void CopyFile(string Source, string Destination)
	{
		if (!File.Exists(Source))
		{
			Console.WriteLine("AzSpeech: File \"" + Source + "\" not found.");
			return;
		}

		try
		{
			Console.WriteLine("AzSpeech: Copying file \"" + Source + "\" to \"" + Destination + "\"");
			File.Copy(Source, Destination, true);
		}
		catch (IOException ex)
		{
			if (File.Exists(Destination))
			{
				// File is already being used. We'll use the existing.
				return;
			}

			Console.WriteLine("AzSpeech: Failed to copy file \"" + Destination + "\" with exception: " + ex.Message);
		}
	}

	private void CopyAndLinkDependencies(string SourceDir, string DestinationDir, bool bAddAsPublicAdditionalLib, bool bAddAsRuntimeDependency, bool bDelayLoadDLL)
	{
		foreach (string Lib in GetLibsList())
		{
			string Source = Path.Combine(SourceDir, Lib);
			string Destination = Path.Combine(DestinationDir, Lib);

			CopyFile(Source, Destination);

			if (bAddAsPublicAdditionalLib)
			{
				PublicAdditionalLibraries.Add(Destination);
			}

			if (bDelayLoadDLL)
			{
				PublicDelayLoadDLLs.Add(Lib);
			}

			if (bAddAsRuntimeDependency)
			{
				RuntimeDependencies.Add(Destination);
			}
		}
	}

	private void DefineBinariesDirectory(string SubDirectory)
	{
		PublicDefinitions.Add(string.Format("AZSPEECH_BINARIES_DIRECTORY=\"{0}\"", SubDirectory.Replace(@"\", @"/")));
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

		Console.WriteLine("AzSpeech: Initializing build for target: Platform: " + Target.Platform.ToString() + "; Architecture: " + Target.Architecture.ToString() + ";");
		Console.WriteLine("AzSpeech: Getting plugin dependencies in directory: \"" + GetPlatformLibsDirectory() + "\"");

		string BinariesSubDirectory = Path.Combine("Binaries", Target.Platform.ToString(), "ThirdParty", Target.Architecture);
		string BinariesDirectory = Path.Combine(PluginDirectory, BinariesSubDirectory);

		// Ensure that the Binaries directory exists
		Directory.CreateDirectory(BinariesDirectory);

		if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.HoloLens)
		{
			string StaticLibFilename = "Microsoft.CognitiveServices.Speech.core.lib";
			string SourceStaticDir = Path.Combine(GetPlatformLibsDirectory(), StaticLibFilename);
			string DestinationStaticDir = Path.Combine(BinariesDirectory, StaticLibFilename);

			// Copy & Link the static library
			CopyFile(SourceStaticDir, DestinationStaticDir);
			PublicAdditionalLibraries.Add(DestinationStaticDir);

			// Copy & Link the Runtime Libraries
			CopyAndLinkDependencies(Path.Combine(GetPlatformLibsDirectory(), "Runtime"), BinariesDirectory, false, true, true);

			if (Target.Platform == UnrealTargetPlatform.HoloLens && isArmArch())
			{
				PublicDefinitions.Add("PLATFORM_HOLOLENS_ARM64=1");
			}

			// Add the definition of the Binaries Sub Directory
			DefineBinariesDirectory(BinariesSubDirectory);
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac || Target.Platform.ToString().ToLower().Contains("linux"))
		{
			// Copy & Link the Dependencies
			CopyAndLinkDependencies(GetPlatformLibsDirectory(), BinariesDirectory, true, true, false);

			// Add the definition of the Binaries Sub Directory
			DefineBinariesDirectory(BinariesSubDirectory);
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// We don't need the 'CopyAndLinkDependencies' here, The UPL file will copy the libs to the Android libs folder
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "AzSpeech_UPL_Android.xml"));

			// Linking both architectures: For some reason (or bug?) Target.Architecture is always empty when building for Android ._.
			foreach (string Lib in GetLibsList())
			{
				PublicAdditionalLibraries.Add(Path.Combine(GetPlatformLibsDirectory(), "arm64-v8a", Lib));
				PublicAdditionalLibraries.Add(Path.Combine(GetPlatformLibsDirectory(), "armeabi-v7a", Lib));
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			// We don't need the 'CopyAndLinkDependencies' here, The UPL file will copy the libs to the iOS libs folder
			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(ModuleDirectory, "AzSpeech_UPL_IOS.xml"));

			foreach (string Lib in GetLibsList())
			{
				PublicAdditionalLibraries.Add(Path.Combine(GetPlatformLibsDirectory(), Lib));
			}
		}
	}
}