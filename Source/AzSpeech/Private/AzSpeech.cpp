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

#define AZSPEECH_SUPPORTED_PLATFORM (PLATFORM_WINDOWS || PLATFORM_ANDROID)

#if WITH_EDITOR
#include <Misc/MessageDialog.h>
#endif

#define LOCTEXT_NAMESPACE "FAzSpeechModule"

#ifdef AZSPEECH_BINARIES_SUBDIRECTORY
FString GetRuntimeLibsDirectory()
{
	const TSharedPtr<IPlugin> PluginInterface = IPluginManager::Get().FindPlugin("AzSpeech");
	FString Directory = PluginInterface->GetBaseDir() / AZSPEECH_BINARIES_SUBDIRECTORY;
	FPaths::NormalizeDirectoryName(Directory);

#if PLATFORM_HOLOLENS
	FPaths::MakePathRelativeTo(Directory, *(FPaths::RootDir() + TEXT("/")));
#endif

	return Directory;
}

TArray<FString> GetWhitelistedRuntimeLibs()
{
	TArray<FString> WhitelistedLibs;

#ifdef AZSPEECH_WHITELISTED_BINARIES
	const FString WhitelistedLibsDef(AZSPEECH_WHITELISTED_BINARIES);
	WhitelistedLibsDef.ParseIntoArray(WhitelistedLibs, TEXT(";"));
#endif

	return WhitelistedLibs;
}

void LogLastError(const FString& FailLib)
{
	const uint32 ErrorID = FPlatformMisc::GetLastError();
	TCHAR ErrorBuffer[MAX_SPRINTF];
	FPlatformMisc::GetSystemErrorMessage(ErrorBuffer, MAX_SPRINTF, ErrorID);

	UE_LOG(LogAzSpeech_Internal, Warning, TEXT("%s: Failed to load runtime library \"%s\": %u (%s)."), *FString(__func__), *FailLib, ErrorID, ErrorBuffer);
}

void FAzSpeechModule::LoadRuntimeLibraries()
{
	const FString BinariesDirectory = GetRuntimeLibsDirectory();
	const TArray<FString> WhitelistedLibs = GetWhitelistedRuntimeLibs();

	UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Loading runtime libraries in directory \"%s\"."), *FString(__func__), *BinariesDirectory);

	FPlatformProcess::PushDllDirectory(*BinariesDirectory);

	for (const FString& RuntimeLib : WhitelistedLibs)
	{
		void* Handle = nullptr;

		// Attempt to load the file more than one time in case of a temporary lock
		constexpr unsigned int MaxAttempt = 5u;
		constexpr float AttemptSleepDelay = 0.5f;
		for (unsigned int Attempt = 1u; !Handle && Attempt <= MaxAttempt; ++Attempt)
		{
			if (Attempt > 1u)
			{
				FPlatformProcess::Sleep(AttemptSleepDelay);
			}

			UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Attempting to load runtime library \"%s\" (%u/%u)."), *FString(__func__), *RuntimeLib, Attempt, MaxAttempt);
			if (Handle = FPlatformProcess::GetDllHandle(*RuntimeLib); Handle)
			{
				break;
			}
		}

		if (!Handle)
		{
			LogLastError(FPaths::Combine(BinariesDirectory, RuntimeLib));
		}
		else
		{
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Loaded runtime library \"%s\"."), *FString(__func__), *RuntimeLib);
			RuntimeLibraries.Add(Handle);
		}
	}

	FPlatformProcess::PopDllDirectory(*BinariesDirectory);
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

#if WITH_EDITOR && !AZSPEECH_SUPPORTED_PLATFORM
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Currently, AzSpeech does not officially support the platform you're using/targeting. If you encounter any issue and can/want to contribute, get in touch! :)\n\nRepository Link: github.com/lucoiso/UEAzSpeech"));
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
#undef AZSPEECH_SUPPORTED_PLATFORM

IMPLEMENT_MODULE(FAzSpeechModule, AzSpeech)
