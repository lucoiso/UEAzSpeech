// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechWavFileSynthesisBase.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeech/AzSpeechInternalFuncs.h"
#include <HAL/PlatformFileManager.h>

void UAzSpeechWavFileSynthesisBase::Activate()
{
#if PLATFORM_ANDROID
	UAzSpeechHelper::CheckAndroidPermission("android.permission.WRITE_EXTERNAL_STORAGE");
#endif

	Super::Activate();
}

void UAzSpeechWavFileSynthesisBase::SetReadyToDestroy()
{
	if (IsTaskReadyToDestroy())
	{
		return;
	}

	if (SynthesisCompleted.IsBound())
	{
		SynthesisCompleted.Clear();;
	}

	Super::SetReadyToDestroy();

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
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("Task: %s (%d); Function: %s; Message: File '%s' deleted successfully."), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *Full_FileName);
		}
		else
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("Task: %s (%d); Function: %s; Message: File '%s' could not be deleted."), *TaskName.ToString(), GetUniqueID(), *FString(__func__), *Full_FileName);
		}
	}
}

void UAzSpeechWavFileSynthesisBase::BroadcastFinalResult()
{
	Super::BroadcastFinalResult();

	if (SynthesisCompleted.IsBound())
	{
		SynthesisCompleted.Broadcast(IsLastResultValid() && UAzSpeechHelper::IsAudioDataValid(GetAudioData()));
		SynthesisCompleted.Clear();
	}
}

bool UAzSpeechWavFileSynthesisBase::StartAzureTaskWork()
{
	if (!Super::StartAzureTaskWork())
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

	const auto AudioConfig = Microsoft::CognitiveServices::Speech::Audio::AudioConfig::FromWavFileOutput(TCHAR_TO_UTF8(*UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName)));
	StartSynthesisWork(AudioConfig);

	return true;
}

void UAzSpeechWavFileSynthesisBase::OnSynthesisUpdate(const std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesisResult>& LastResult)
{
	Super::OnSynthesisUpdate(LastResult);

	if (!UAzSpeechTaskBase::IsTaskStillValid(this))
	{
		return;
	}

	if (CanBroadcastWithReason(LastResult->Reason))
	{
		FScopeLock Lock(&Mutex);

		BroadcastFinalResult();
	}
}