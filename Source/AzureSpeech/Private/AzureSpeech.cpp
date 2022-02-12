// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/AzureSpeech

#include "AzureSpeech.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FAzureSpeechModule"

void FAzureSpeechModule::StartupModule()
{
#if PLATFORM_WINDOWS
	const FString PreDir = FPaths::Combine(*IPluginManager::Get().FindPlugin("AzureSpeech")->GetBaseDir(),
	                                       TEXT("Source/ThirdParty/AzureWrapper/lib/"));

	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.core.dll", CoreDLL);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.audio.sys.dll", AudioDLL);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.kws.dll", KwsDLL);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.lu.dll", LuDLL);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.mas.dll", MasDLL);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.silk_codec.dll", SilkDLL);
	LoadDependency(PreDir + "Microsoft.CognitiveServices.Speech.extension.codec.dll", CodecDLL);
#else
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("AzureSpeechError", "Failed to load AzureSpeech - Currently supports only Win64 builds"));
#endif
}

void FAzureSpeechModule::ShutdownModule()
{
	FreeDependency(CoreDLL);
	FreeDependency(AudioDLL);
	FreeDependency(KwsDLL);
	FreeDependency(LuDLL);
	FreeDependency(MasDLL);
	FreeDependency(SilkDLL);
	FreeDependency(CodecDLL);
}

void FAzureSpeechModule::FreeDependency(void*& Handle)
{
	if (Handle != nullptr)
	{
		FPlatformProcess::FreeDllHandle(Handle);
		Handle = nullptr;
	}
}

void FAzureSpeechModule::LoadDependency(const FString& Path, void*& Handle)
{
	Handle = FPlatformProcess::GetDllHandle(*Path);

	if (Handle == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load required library %s. Plug-in will not be functional."), *Path);

		// const FText MsgInfo = ("AzureSpeechError", FText::FromString(FString("Failed to load AzureSpeech library: ") + Path));
		// FMessageDialog::Open(EAppMsgType::Ok, MsgInfo);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAzureSpeechModule, AzureSpeech)
