// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToWavAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"
#include "Async/Async.h"

UTextToWavAsync* UTextToWavAsync::TextToWav(const UObject* WorldContextObject, const FString& TextToConvert, const FString& FilePath, const FString& FileName, const FString& VoiceName, const FString& LanguageId)
{
	UTextToWavAsync* const NewAsyncTask = NewObject<UTextToWavAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = TextToConvert;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->bIsSSMLBased = false;

	return NewAsyncTask;
}

void UTextToWavAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.WRITE_EXTERNAL_STORAGE");
#endif

	Super::Activate();
}

bool UTextToWavAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (HasEmptyParam(SynthesisText, VoiceName, LanguageId, FilePath, FileName))
	{
		return false;
	}

	if (!UAzSpeechHelper::CreateNewDirectory(FilePath))
	{
		return false;
	}

	const std::string InFilePath = TCHAR_TO_UTF8(*UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName));
	const auto AudioConfig = Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromWavFileOutput(InFilePath);

	if (!InitializeSynthesizer(AudioConfig))
	{
		return false;
	}

	StartSynthesisWork();

	return true;
}

void UTextToWavAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(SynthesisEventArgs.Result->Reason))
	{
		AsyncTask(ENamedThreads::GameThread, [=] { SynthesisCompleted.Broadcast(bLastResultIsValid); });
	}
}