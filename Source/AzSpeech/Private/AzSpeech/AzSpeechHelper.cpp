// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeech.h"
#include "Sound/SoundWave.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "DesktopPlatformModule.h"
#include "Kismet/GameplayStatics.h"

#if PLATFORM_ANDROID
#include "AndroidPermissionFunctionLibrary.h"
#endif

FString UAzSpeechHelper::QualifyPath(const FString& Path)
{
	FString Output = Path;
	if (*Path.end() != '/')
	{
		Output += '/';
	}

	return Output;
}

FString UAzSpeechHelper::QualifyFileExtension(const FString& Path, const FString& Name, const FString& Extension)
{
	if (Path.IsEmpty() || Name.IsEmpty() || Extension.IsEmpty())
	{
		return FString();
	}

	const FString LocalPath = QualifyPath(Path);
	const FString LocalExtension = Extension.Contains(".") ? Extension : "." + Extension;

	FString LocalName = Name;
	if (!Name.Right(Name.Len() - LocalExtension.Len()).Contains(LocalExtension))
	{
		LocalName += LocalExtension;
	}

	const FString QualifiedName = LocalPath + LocalName;

	UE_LOG(LogAzSpeech, Log, TEXT("AzSpeech - %s: Qualified %s file path: %s"), *FString(__func__), *LocalExtension.ToUpper(), *QualifiedName);

	return QualifiedName;
}

USoundWave* UAzSpeechHelper::ConvertFileToSoundWave(const FString& FilePath, const FString& FileName)
{
	if (FilePath.IsEmpty() || FileName.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: FilePath or FileName is empty"), *FString(__func__));
	}

	else if (const FString Full_FileName = QualifyWAVFileName(FilePath, FileName); 
		FPlatformFileManager::Get().GetPlatformFile().FileExists(*Full_FileName))
	{
#if PLATFORM_ANDROID
		CheckAndroidPermission("android.permission.READ_EXTERNAL_STORAGE");
#endif

		if (TArray<uint8> RawData; 
			FFileHelper::LoadFileToArray(RawData, *Full_FileName, FILEREAD_NoFail))
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(__func__));
			return ConvertStreamToSoundWave(RawData);
		}
		// else
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed to load file"), *FString(__func__));
	}

	UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Cannot find the specified file"), *FString(__func__));
	return nullptr;
}

USoundWave* UAzSpeechHelper::ConvertStreamToSoundWave(const TArray<uint8>& RawData)
{
#if ENGINE_MAJOR_VERSION >= 5
	if (RawData.IsEmpty())
#else
	if (RawData.Num() == 0)
#endif
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: RawData is empty"), *FString(__func__));
	}
	else if (USoundWave* const SoundWave = NewObject<USoundWave>())
	{
		FWaveModInfo WaveInfo;
		WaveInfo.ReadWaveInfo(RawData.GetData(), RawData.Num());

		const int32 ChannelCount = *WaveInfo.pChannels;
		const int32 SizeOfSample = *WaveInfo.pBitsPerSample / 8;
		const int32 NumSamples = WaveInfo.SampleDataSize / SizeOfSample;
		const int32 NumFrames = NumSamples / ChannelCount;

		SoundWave->Duration = NumFrames / *WaveInfo.pSamplesPerSec;
		SoundWave->NumChannels = ChannelCount;
		SoundWave->TotalSamples = *WaveInfo.pSamplesPerSec * SoundWave->Duration;
		SoundWave->SetSampleRate(*WaveInfo.pSamplesPerSec);

#if ENGINE_MAJOR_VERSION >= 5
		SoundWave->SetImportedSampleRate(*WaveInfo.pSamplesPerSec);
#endif

		SoundWave->RawPCMDataSize = WaveInfo.SampleDataSize;
		SoundWave->RawPCMData = static_cast<uint8*>(FMemory::Malloc(WaveInfo.SampleDataSize));

		FMemory::Memcpy(SoundWave->RawPCMData, WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);

		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(__func__));
		return SoundWave;
	}

	UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Cannot create a new Sound Wave"), *FString(__func__));
	return nullptr;
}

FString UAzSpeechHelper::LoadXMLToString(const FString& FilePath, const FString& FileName)
{
	if (FilePath.IsEmpty() || FileName.IsEmpty())
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: FilePath or FileName is empty"), *FString(__func__));
	}
	else if (const FString Full_FileName = QualifyXMLFileName(FilePath, FileName); 
		FPlatformFileManager::Get().GetPlatformFile().FileExists(*Full_FileName))
	{
		if (FString OutputStr; 
			FFileHelper::LoadFileToString(OutputStr, *Full_FileName))
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: %s loaded"), *FString(__func__), *Full_FileName);
			return OutputStr;
		}
	}

	UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed to load file"), *FString(__func__));
	return FString();
}

bool UAzSpeechHelper::CreateNewDirectory(const FString& Path, const bool bCreateParents)
{
	bool bOutput = FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*Path);

	if (!bOutput)
	{
		UE_LOG(LogAzSpeech, Warning, TEXT("AzSpeech - %s: Folder does not exist, trying to create a new with the specified path"), *FString(__func__));

		bOutput = bCreateParents ? FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*Path)
								 : FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*Path);
	}

	if (bOutput)
	{
		UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(__func__));
	}
	else
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed to create a new folder"), *FString(__func__));
	}

	return bOutput;
}

FString UAzSpeechHelper::OpenDesktopFolderPicker()
{
	FString OutputPath;

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	if (IDesktopPlatform* const DesktopPlatform = FDesktopPlatformModule::Get())
	{
		if (DesktopPlatform->OpenDirectoryDialog(nullptr, TEXT("Select a folder"), TEXT(""), OutputPath))
		{
			UE_LOG(LogAzSpeech, Display, TEXT("AzSpeech - %s: Result: Success"), *FString(__func__));
		}
		else
		{
			UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed to open a folder picker or the user cancelled the operation"), *FString(__func__));
		}
	}
	else
	{
		UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Result: Failed to get Desktop Platform"), *FString(__func__));
	}
#else
	UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Platform %s is not supported"), *FString(__func__), *UGameplayStatics::GetPlatformName());
#endif

	return OutputPath;
}

void UAzSpeechHelper::CheckAndroidPermission([[maybe_unused]] const FString& InPermissionStr)
{
#if PLATFORM_ANDROID
	if (!UAndroidPermissionFunctionLibrary::CheckPermission(InPermissionStr))
	{
		UAndroidPermissionFunctionLibrary::AcquirePermissions({ InPermissionStr });
	}
#else
	UE_LOG(LogAzSpeech, Error, TEXT("AzSpeech - %s: Platform %s is not supported"), *FString(__func__), *UGameplayStatics::GetPlatformName());
#endif
}
