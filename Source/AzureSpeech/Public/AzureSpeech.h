// Lucas Vilas-Boas - 2022

#pragma once

/**
 *
 */

class FAzureSpeechModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void* CoreDLL;
	void* AudioDLL;
	void* CodecDLL;
	void* KwsDLL;
	void* LuDLL;
	void* SilkDLL;

	void FreeDependency(void*& Handle);
	bool LoadDependency(const FString& Name, void*& Handle);
};
