// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#if ENGINE_MAJOR_VERSION < 5
#include "GenericPlatform/GenericPlatformProcess.h"
#endif

#define LOCTEXT_NAMESPACE "FAzSpeechModule"

DEFINE_LOG_CATEGORY(LogAzSpeech);

void FAzSpeechModule::StartupModule()
{
#if PLATFORM_WINDOWS
	const FString PreDir = FPaths::Combine(
		*IPluginManager::Get().FindPlugin("AzSpeech")->GetBaseDir(),
		TEXT("Source/ThirdParty/AzureWrapper/libs/Win/Runtime/"));

	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.core.dll", CoreRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.audio.sys.dll", AudioRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.kws.dll", KwsRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.lu.dll", LuRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.mas.dll", MasRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.silk_codec.dll", SilkRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.codec.dll", CodecRuntimeLib);
#endif
}

void FAzSpeechModule::ShutdownModule()
{
	FreeDependency(CoreRuntimeLib);
	FreeDependency(AudioRuntimeLib);
	FreeDependency(KwsRuntimeLib);
	FreeDependency(LuRuntimeLib);
	FreeDependency(MasRuntimeLib);
	FreeDependency(SilkRuntimeLib);
	FreeDependency(CodecRuntimeLib);
}

void FAzSpeechModule::FreeDependency(void*& Handle)
{
	if (Handle != nullptr)
	{
		FPlatformProcess::FreeDllHandle(Handle);
		Handle = nullptr;
	}
}

void FAzSpeechModule::LoadDependency(const FString& Path, void*& Handle)
{
	Handle = FPlatformProcess::GetDllHandle(*Path);

	if (Handle == nullptr)
	{
		UE_LOG(LogAzSpeech, Warning,
		       TEXT("AzSpeech - %s: Failed to load required library %s. Plug-in will not be functional."),
		       *FString(__func__), *Path);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAzSpeechModule, AzSpeech)
