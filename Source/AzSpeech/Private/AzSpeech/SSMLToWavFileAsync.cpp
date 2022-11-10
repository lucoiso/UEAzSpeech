// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToWavFileAsync.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"
#include "Async/Async.h"

USSMLToWavFileAsync* USSMLToWavFileAsync::SSMLToWavFile(const UObject* WorldContextObject, const FString& SSMLString, const FString& FilePath, const FString& FileName)
{
	USSMLToWavFileAsync* const NewAsyncTask = NewObject<USSMLToWavFileAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SynthesisText = SSMLString;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->bIsSSMLBased = true;

	return NewAsyncTask;
}

void USSMLToWavFileAsync::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.WRITE_EXTERNAL_STORAGE");
#endif

	Super::Activate();
}

bool USSMLToWavFileAsync::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (AzSpeech::Internal::HasEmptyParam(SynthesisText, FilePath, FileName))
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

void USSMLToWavFileAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(SynthesisEventArgs.Result->Reason))
	{
		AsyncTask(ENamedThreads::GameThread, [=] { SynthesisCompleted.Broadcast(IsLastResultValid()); });
	}
}