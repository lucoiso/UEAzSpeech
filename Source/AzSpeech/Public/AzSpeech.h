// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "Modules/ModuleInterface.h"

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
	void* CoreDLL = nullptr;
	void* AudioDLL = nullptr;
	void* KwsDLL = nullptr;
	void* LuDLL = nullptr;
	void* MasDLL = nullptr;
	void* SilkDLL = nullptr;
	void* CodecDLL = nullptr;

	static void FreeDependency(void*& Handle);
	static void LoadDependency(const FString& Path, void*& Handle);
};
