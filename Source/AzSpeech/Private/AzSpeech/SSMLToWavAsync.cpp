// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/SSMLToWavAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"

USSMLToWavAsync* USSMLToWavAsync::SSMLToWav(const UObject* WorldContextObject, const FString& SSMLString, const FString& FilePath, const FString& FileName)
{
	USSMLToWavAsync* const NewAsyncTask = NewObject<USSMLToWavAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->SSMLString = SSMLString;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;

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

	if (SSMLString.IsEmpty() || FilePath.IsEmpty() || FileName.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Missing parameters"), *FString(__func__));
		return false;
	}

	if (!UAzSpeechHelper::CreateNewDirectory(FilePath))
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Failed to create directory"), *FString(__func__));
		return false;
	}

	UE_LOG(LogAzSpeech, Display, TEXT("%s: Initializing task"), *FString(__func__));

	const std::string InSSML = TCHAR_TO_UTF8(*SSMLString);
	const std::string InFilePath = TCHAR_TO_UTF8(*UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName));

	const auto AudioConfig = AudioConfig::FromWavFileOutput(InFilePath);
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig);

	if (!SynthesizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Failed to proceed with task: SynthesizerObject is null"), *FString(__func__));
		return false;
	}

	ApplyExtraSettings();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]
	{
		SynthesizerObject->SpeakSsmlAsync(InSSML).wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
	});

	return true;
}

void USSMLToWavAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (SynthesisEventArgs.Result->Reason == ResultReason::SynthesizingAudioCompleted)
	{
		SynthesisCompleted.Broadcast(true);
	}
}