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
	FString LibsDirectory = FPaths::Combine(*PluginInterface->GetBaseDir(), AZSPEECH_BINARIES_SUBDIRECTORY);

#if PLATFORM_HOLOLENS
	FPaths::MakePathRelativeTo(LibsDirectory, *(FPaths::RootDir() + TEXT("/")));
#endif

	return LibsDirectory;
}

TArray<FString> GetRuntimeLibraries()
{
	TArray<FString> LibsArray;

#if PLATFORM_WINDOWS || PLATFORM_HOLOLENS
	LibsArray = {
		"Microsoft.CognitiveServices.Speech.core.dll",
		"Microsoft.CognitiveServices.Speech.extension.audio.sys.dll",
		"Microsoft.CognitiveServices.Speech.extension.kws.dll",
		"Microsoft.CognitiveServices.Speech.extension.lu.dll",
		"Microsoft.CognitiveServices.Speech.extension.mas.dll",

#ifndef PLATFORM_HOLOLENS_ARM64
		"Microsoft.CognitiveServices.Speech.extension.codec.dll"
#endif
	};

#elif PLATFORM_MAC || PLATFORM_MAC_ARM64
	LibsArray = { "libMicrosoft.CognitiveServices.Speech.core.dylib" };

#elif PLATFORM_LINUX || PLATFORM_LINUXARM64	
	LibsArray = {
		"libMicrosoft.CognitiveServices.Speech.core.so",
		"libMicrosoft.CognitiveServices.Speech.extension.audio.sys.so",
		"libMicrosoft.CognitiveServices.Speech.extension.kws.so",
		"libMicrosoft.CognitiveServices.Speech.extension.lu.so",
		"libMicrosoft.CognitiveServices.Speech.extension.mas.so",
		"libMicrosoft.CognitiveServices.Speech.extension.codec.so"
	};
#endif

	return LibsArray;
}

void LoadRuntimeLibraries()
{
	const FString Path = GetRuntimeLibsDirectory();
	const TArray<FString> Libs = GetRuntimeLibraries();

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Loading runtime libraries in path \"%s\"."), *FString(__func__), *Path);

	FPlatformProcess::PushDllDirectory(*Path);
	for (const FString& Lib : Libs)
	{
		if (!FPlatformProcess::GetDllHandle(*Lib))
		{
			UE_LOG(LogAzSpeech_Internal, Warning, TEXT("%s: Failed to load runtime library \"%s\"."), *FString(__func__), *Lib);
		}
	}
	FPlatformProcess::PopDllDirectory(*Path);
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
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAzSpeechModule, AzSpeech)
