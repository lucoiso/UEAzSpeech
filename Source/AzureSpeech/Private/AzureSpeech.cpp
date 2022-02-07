#include "AzureSpeech.h"
#include "Core.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FAzureSpeechModule"

void FAzureSpeechModule::StartupModule()
{
	LoadDependency("Microsoft.CognitiveServices.Speech.core.dll", CoreDLL);
	LoadDependency("Microsoft.CognitiveServices.Speech.extension.audio.sys.dll", AudioDLL);
	LoadDependency("Microsoft.CognitiveServices.Speech.extension.codec.dll", CodecDLL);
	LoadDependency("Microsoft.CognitiveServices.Speech.extension.kws.dll", KwsDLL);
	LoadDependency("Microsoft.CognitiveServices.Speech.extension.lu.dll", LuDLL);
	LoadDependency("Microsoft.CognitiveServices.Speech.extension.silk_codec.dll", SilkDLL);
}

void FAzureSpeechModule::ShutdownModule()
{
	FreeDependency(CoreDLL);
	FreeDependency(AudioDLL);
	FreeDependency(CodecDLL);
	FreeDependency(KwsDLL);
	FreeDependency(LuDLL);
	FreeDependency(SilkDLL);
}

void FAzureSpeechModule::FreeDependency(void*& Handle)
{
	if (Handle != nullptr)
	{
		FPlatformProcess::FreeDllHandle(Handle);
		Handle = nullptr;
	}
}

bool FAzureSpeechModule::LoadDependency(const FString& Name, void*& Handle)
{
	const FString filePath = FPaths::Combine(*FPaths::ProjectDir(), TEXT("Binaries/Win64"), *Name);
	Handle = FPlatformProcess::GetDllHandle(*filePath);

	if (Handle == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load required library %s. Plug-in will not be functional."), *Name);
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAzureSpeechModule, AzureSpeech)
