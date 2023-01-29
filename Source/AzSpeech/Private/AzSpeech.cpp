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

#ifdef AZSPEECH_BINARIES_SUBDIRECTORY
FString GetRuntimeLibsDirectory()
{
	const TSharedPtr<IPlugin> PluginInterface = IPluginManager::Get().FindPlugin("AzSpeech");
	return PluginInterface->GetBaseDir() / AZSPEECH_BINARIES_SUBDIRECTORY;
}

FString GetRuntimeLibsType()
{
#if PLATFORM_WINDOWS || PLATFORM_HOLOLENS
	return "*.dll";

#elif PLATFORM_MAC || PLATFORM_MAC_ARM64
	return "*.dylib";

#elif PLATFORM_LINUX || PLATFORM_LINUXARM64
	return "*.so";
#endif

	return FString();
}

void FAzSpeechModule::LoadRuntimeLibraries()
{
	const FString BinDir = GetRuntimeLibsDirectory();
	const FString LibType = GetRuntimeLibsType();

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Searching for runtime libraries with extension \"%s\" in directory \"%s\"."), *FString(__func__), *LibType, *BinDir);

	TArray<FString> Libs;
	IFileManager::Get().FindFilesRecursive(Libs, *BinDir, *LibType, true, false);

	for (const FString& FoundLib : Libs)
	{
		void* Handle = FPlatformProcess::GetDllHandle(*FoundLib);

		if (!Handle)
		{
			UE_LOG(LogAzSpeech_Internal, Warning, TEXT("%s: Failed to load runtime library \"%s\"."), *FString(__func__), *FoundLib);
			continue;
		}

		UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Loaded runtime library \"%s\"."), *FString(__func__), *FoundLib);
		RuntimeLibraries.Add(Handle);
	}
}

void FAzSpeechModule::UnloadRuntimeLibraries()
{
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Unloading runtime libraries."), *FString(__func__));

	for (void*& Handle : RuntimeLibraries)
	{
		if (!Handle)
		{
			continue;
		}

		FPlatformProcess::FreeDllHandle(Handle);
		Handle = nullptr;
	}

	RuntimeLibraries.Empty();
}
#endif

void FAzSpeechModule::StartupModule()
{
	const TSharedPtr<IPlugin> PluginInterface = IPluginManager::Get().FindPlugin("AzSpeech");
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Initializing plugin %s version %s."), *PluginInterface->GetFriendlyName(), *PluginInterface->GetDescriptor().VersionName);

#if !PLATFORM_ANDROID && !PLATFORM_IOS
	if (FPaths::DirectoryExists(UAzSpeechHelper::GetAzSpeechLogsBaseDir()))
	{
		IFileManager::Get().DeleteDirectory(*UAzSpeechHelper::GetAzSpeechLogsBaseDir(), true, true);
	}
#endif

#ifdef AZSPEECH_BINARIES_SUBDIRECTORY
	LoadRuntimeLibraries();
#endif
}

void FAzSpeechModule::ShutdownModule()
{
	const TSharedPtr<IPlugin> PluginInterface = IPluginManager::Get().FindPlugin("AzSpeech");
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("Shutting down plugin %s version %s."), *PluginInterface->GetFriendlyName(), *PluginInterface->GetDescriptor().VersionName);

#ifdef AZSPEECH_BINARIES_SUBDIRECTORY
	UnloadRuntimeLibraries();
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAzSpeechModule, AzSpeech)
