// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

using System.IO;
using UnrealBuildTool;

public class AzureWrapper : ModuleRules
{
	public AzureWrapper(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		bEnableExceptions = true;

		PublicIncludePaths.AddRange(
            new[]
            {
                Path.Combine(ModuleDirectory, "include")
            }
        );

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Microsoft.CognitiveServices.Speech.core.lib"));

			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.core.dll");
			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.extension.audio.sys.dll");
			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.extension.kws.dll");
			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.extension.lu.dll");
			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.extension.mas.dll");
			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.extension.silk_codec.dll");
			PublicDelayLoadDLLs.Add("Microsoft.CognitiveServices.Speech.extension.codec.dll");

			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "lib", "Microsoft.CognitiveServices.Speech.core.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "lib", "Microsoft.CognitiveServices.Speech.extension.audio.sys.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "lib", "Microsoft.CognitiveServices.Speech.extension.kws.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "lib", "Microsoft.CognitiveServices.Speech.extension.lu.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "lib", "Microsoft.CognitiveServices.Speech.extension.mas.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "lib", "Microsoft.CognitiveServices.Speech.extension.silk_codec.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "lib", "Microsoft.CognitiveServices.Speech.extension.codec.dll"));
        }
	}
}
