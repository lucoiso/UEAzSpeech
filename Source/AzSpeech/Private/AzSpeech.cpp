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

#define PLATFORM_LINUXARM64 PLATFORM_LINUXAARCH64
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
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.lu.dll", LuRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.mas.dll", MasRuntimeLib);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.codec.dll", CodecRuntimeLib);

#elif PLATFORM_MAC || PLATFORM_MAC_ARM64
#if PLATFORM_MAC_ARM64
	const FString PreDir = FPaths::Combine(*PluginInterface->GetBaseDir(), TEXT("Source/ThirdParty/AzureWrapper/libs/Mac/Arm64/"));
#else
	const FString PreDir = FPaths::Combine(*PluginInterface->GetBaseDir(), TEXT("Source/ThirdParty/AzureWrapper/libs/Mac/x64/"));
#endif

	LoadDependency(PreDir + "libMicrosoft.CognitiveServices.Speech.core.dylib", CoreRuntimeLib);

#elif PLATFORM_LINUX || PLATFORM_LINUXARM64
#if PLATFORM_LINUXARM64
	const FString PreDir = FPaths::Combine(*PluginInterface->GetBaseDir(), TEXT("Source/ThirdParty/AzureWrapper/libs/Linux/Arm64/"));
#else
	const FString PreDir = FPaths::Combine(*PluginInterface->GetBaseDir(), TEXT("Source/ThirdParty/AzureWrapper/libs/Linux/x64/"));
#endif

	LoadDependency(PreDir + "libMicrosoft.CognitiveServices.Speech.core.so", CoreRuntimeLib);
	LoadDependency(PreDir + "libMicrosoft.CognitiveServices.Speech.extension.audio.sys.so", AudioRuntimeLib);
	LoadDependency(PreDir + "libMicrosoft.CognitiveServices.Speech.extension.kws.so", KwsRuntimeLib);
	LoadDependency(PreDir + "libMicrosoft.CognitiveServices.Speech.extension.lu.so", LuRuntimeLib);
	LoadDependency(PreDir + "libMicrosoft.CognitiveServices.Speech.extension.mas.so", MasRuntimeLib);
	LoadDependency(PreDir + "libMicrosoft.CognitiveServices.Speech.extension.codec.so", CodecRuntimeLib);
#endif

	if (FPaths::DirectoryExists(UAzSpeechHelper::GetAzSpeechLogsBaseDir()))
	{
		IFileManager::Get().DeleteDirectory(*UAzSpeechHelper::GetAzSpeechLogsBaseDir(), true, true);
	}
}

void FAzSpeechModule::ShutdownModule()
{
	FreeDependency(CoreRuntimeLib);
	
#if !PLATFORM_MAC
	FreeDependency(AudioRuntimeLib);
	FreeDependency(KwsRuntimeLib);
	FreeDependency(LuRuntimeLib);
	FreeDependency(MasRuntimeLib);
	FreeDependency(CodecRuntimeLib);
#endif
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
