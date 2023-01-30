// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech.h"
#include "LogAzSpeech.h"
#include "AzSpeech/AzSpeechHelper.h"
#include <Modules/ModuleManager.h>
#include <Interfaces/IPluginManager.h>
#include <Misc/Paths.h>
#include <HAL/FileManager.h>

#if ENGINE_MAJOR_VERSION < 5
#include <GenericPlatform/GenericPlatformProcess.h>
#endif

#define LOCTEXT_NAMESPACE "FAzSpeechModule"

void FAzSpeechModule::StartupModule()
{
	const TSharedPtr<IPlugin> PluginInterface = IPluginManager::Get().FindPlugin("AzSpeech");
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Initializing plugin %s version %s."), *PluginInterface->GetFriendlyName(), *PluginInterface->GetDescriptor().VersionName);

#if PLATFORM_WINDOWS
	const FString PreDir = FPaths::Combine(*PluginInterface->GetBaseDir(), TEXT("Source/ThirdParty/AzureWrapper/libs/Win/Runtime/"));

	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.core.dll", CoreRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.audio.sys.dll", AudioRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.kws.dll", KwsRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.kws.ort.dll", KwsOrtRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.lu.dll", LuRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.mas.dll", MasRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.codec.dll", CodecRuntimeLib);
#endif

	if (FPaths::DirectoryExists(UAzSpeechHelper::GetAzSpeechLogsBaseDir()))
	{
		IFileManager::Get().DeleteDirectory(*UAzSpeechHelper::GetAzSpeechLogsBaseDir(), true, true);
	}
}

void FAzSpeechModule::ShutdownModule()
{
	FreeDependency(CoreRuntimeLib);
	FreeDependency(AudioRuntimeLib);
	FreeDependency(KwsRuntimeLib);
	FreeDependency(KwsOrtRuntimeLib);
	FreeDependency(LuRuntimeLib);
	FreeDependency(MasRuntimeLib);
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
		UE_LOG(LogAzSpeech_Internal, Warning, TEXT("%s: Failed to load library %s."), *FString(__func__), *Path);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAzSpeechModule, AzSpeech)
