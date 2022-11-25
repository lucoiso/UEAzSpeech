// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Bases/AzSpeechWavFileSynthesisBase.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include "Async/Async.h"

void UAzSpeechWavFileSynthesisBase::Activate()
{
	bNullifySynthesizerObjectOnStop = true;

#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.WRITE_EXTERNAL_STORAGE");
#endif

	Super::Activate();
}

void UAzSpeechWavFileSynthesisBase::StopAzSpeechTask()
{
	Super::StopAzSpeechTask();
		
	if (IsLastResultValid())
	{
		return;
	}

	// If the task was cancelled due to SDK errors, we need to delete the generated invalid file
	if (const FString Full_FileName = UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName);
		FPlatformFileManager::Get().GetPlatformFile().FileExists(*Full_FileName))
	{
		const bool bDeleteResult = FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*Full_FileName);

		if (bDeleteResult)
		{
			UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%s); Function: %s; Message: File %s deleted successfully."), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *FString::FromInt(GetUniqueID()), *Full_FileName);
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("Task: %s (%s); Function: %s; Message: File %s could not be deleted."), *TaskName.ToString(), *FString::FromInt(GetUniqueID()), *FString(__func__), *Full_FileName);
		}
	}
}

void UAzSpeechWavFileSynthesisBase::BroadcastFinalResult()
{	
	SynthesisCompleted.Broadcast(IsLastResultValid() && UAzSpeechHelper::IsAudioDataValid(GetLastSynthesizedAudioData()));
	Super::BroadcastFinalResult();
}

bool UAzSpeechWavFileSynthesisBase::StartAzureTaskWork_Internal()
{
	if (!Super::StartAzureTaskWork_Internal())
	{
		return false;
	}

	if (AzSpeech::Internal::HasEmptyParam(SynthesisText, VoiceName, LanguageId, FilePath, FileName))
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

void UAzSpeechWavFileSynthesisBase::OnSynthesisUpdate()
{
	Super::OnSynthesisUpdate();

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(LastSynthesisResult->Reason))
	{
		BroadcastFinalResult();

		// Free the process handle
		SynthesizerObject = nullptr;
	}
}