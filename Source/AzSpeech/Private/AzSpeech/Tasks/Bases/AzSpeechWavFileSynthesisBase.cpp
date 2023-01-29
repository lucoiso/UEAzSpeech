// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Bases/AzSpeechWavFileSynthesisBase.h"
#include "AzSpeech/AzSpeechHelper.h"
#include "LogAzSpeech.h"

#if PLATFORM_HOLOLENS
#include <Windows/AllowWindowsPlatformTypes.h>
#include <fileapi.h>
#include <Windows/HideWindowsPlatformTypes.h>
#endif

#if ENGINE_MAJOR_VERSION < 5
#include <HAL/PlatformFilemanager.h>
#else
#include <HAL/PlatformFileManager.h>
#endif

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
	
	const FString Full_FileName = UAzSpeechHelper::QualifyWAVFileName(FilePath, FileName);

	if (IsLastResultValid() && FPlatformFileManager::Get().GetPlatformFile().FileSize(*Full_FileName) > 0)
	{
		return;
	}

	// If the task was cancelled due to SDK errors, we need to delete the generated invalid file
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*Full_FileName))
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

	FScopeLock Lock(&Mutex);

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

	if (HasEmptyParameters(SynthesisText, FilePath, FileName) ||
		(!bIsSSMLBased && HasEmptyParameters(VoiceName, LanguageID)))
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