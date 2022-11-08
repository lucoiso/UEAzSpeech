// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToWavAsync.h"
#include "AzSpeech.h"
#include "Async/Async.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"

UTextToWavAsync* UTextToWavAsync::TextToWav(const UObject* WorldContextObject, const FString& TextToConvert, const FString& FilePath, const FString& FileName, const FString& VoiceName, const FString& LanguageId)
{
	UTextToWavAsync* const NewAsyncTask = NewObject<UTextToWavAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->TextToConvert = TextToConvert;
	NewAsyncTask->FilePath = FilePath;
	NewAsyncTask->FileName = FileName;
	NewAsyncTask->VoiceName = AzSpeech::Internal::GetVoiceName(VoiceName);
	NewAsyncTask->LanguageID = AzSpeech::Internal::GetLanguageID(LanguageId);

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

	if (TextToConvert.IsEmpty() || VoiceName.IsEmpty() || FilePath.IsEmpty() || FileName.IsEmpty() || LanguageID.IsEmpty())
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

	const std::string InText = TCHAR_TO_UTF8(*TextToConvert);
	const std::string InLanguage = TCHAR_TO_UTF8(*LanguageID);
	const std::string InVoice = TCHAR_TO_UTF8(*VoiceName);
	const std::string InFilePath = TCHAR_TO_UTF8(*UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName));

	const auto AudioConfig = AudioConfig::FromWavFileOutput(InFilePath);
	SynthesizerObject = AzSpeech::Internal::GetAzureSynthesizer(AudioConfig, InLanguage, InVoice);

	if (!SynthesizerObject)
	{
		UE_LOG(LogAzSpeech, Error, TEXT("%s: Failed to proceed with task: SynthesizerObject is null"), *FString(__func__));
		return false;
	}

	ApplyExtraSettings();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]
	{
		SynthesizerObject->SpeakTextAsync(InText).wait_for(std::chrono::seconds(AzSpeech::Internal::GetTimeout()));
	});

	return true;
}

void UTextToWavAsync::OnSynthesisUpdate(const Microsoft::CognitiveServices::Speech::SpeechSynthesisEventArgs& SynthesisEventArgs)
{
	Super::OnSynthesisUpdate(SynthesisEventArgs);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (SynthesisEventArgs.Result->Reason != ResultReason::SynthesizingAudio)
	{
		SynthesisCompleted.Broadcast(bLastResultIsValid);
	}
}