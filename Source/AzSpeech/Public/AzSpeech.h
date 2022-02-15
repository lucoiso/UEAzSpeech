// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "Modules/ModuleManager.h"

/**
 *
 */

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

	void FreeDependency(void*& Handle);
	void LoadDependency(const FString& Path, void*& Handle);
};
