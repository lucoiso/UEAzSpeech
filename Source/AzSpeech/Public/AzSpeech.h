// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MAJOR_VERSION >= 5
#include "Modules/ModuleInterface.h"
#else
#include "Modules/ModuleManager.h"
#endif

/**
 *
 */

DECLARE_LOG_CATEGORY_EXTERN(LogAzSpeech, Display, Verbose);

class FAzSpeechModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void* CoreRuntimeLib = nullptr;
	void* AudioRuntimeLib = nullptr;
	void* KwsRuntimeLib = nullptr;
	void* LuRuntimeLib = nullptr;
	void* MasRuntimeLib = nullptr;
	void* SilkCodecRuntimeLib = nullptr;
	void* CodecRuntimeLib = nullptr;

	static void FreeDependency(void*& Handle);
	static void LoadDependency(const FString& Path, void*& Handle);
};
