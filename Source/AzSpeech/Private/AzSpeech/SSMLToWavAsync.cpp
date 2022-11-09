// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToWavAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"
#include "Async/Async.h"

USSMLToWavAsync* USSMLToWavAsync::SSMLToWav(const UObject* WorldContextObject, const FString& SSMLString, const FString& FilePath, const FString& FileName)
{
	USSMLToWavAsync* const NewAsyncTask = NewObject<USSMLToWavAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SSMLString;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->bIsSSMLBased = true;

	return NewAsyncTask;
}

void USSMLToWavAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.WRITE_EXTERNAL_STORAGE");
#endif

	Super::Activate();
}

bool USSMLToWavAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (HasEmptyParam(SynthesisText, FilePath, FileName))
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

void USSMLToWavAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
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