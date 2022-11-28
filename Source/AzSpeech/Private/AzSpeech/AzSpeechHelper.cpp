// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechHelper.h"
#include "LogAzSpeech.h"
#include "Sound/SoundWave.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "DesktopPlatformModule.h"
#include "Kismet/GameplayStatics.h"
#include "AzSpeechInternalFuncs.h"

#if PLATFORM_ANDROID
#include "AndroidPermissionFunctionLibrary.h"
#endif

FString UAzSpeechHelper::QualifyPath(const FString& Path)
{
	FString Output = Path;
	if (!Output.EndsWith("/") && !Output.EndsWith("\""))
	{
		Output += '/';
	}

	UE_LOG(LogAzSpeech_Internal, Log, TEXT("%s: Qualified directory path: %s"), *FString(__func__), *Output);

	return Output;
}

FString UAzSpeechHelper::QualifyFileExtension(const FString& Path, const FString& Name, const FString& Extension)
{
	if (AzSpeech::Internal::HasEmptyParam(Path, Name, Extension))
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

	UE_LOG(LogAzSpeech_Internal, Log, TEXT("%s: Qualified %s file path: %s"), *FString(__func__), *LocalExtension.ToUpper(), *QualifiedName);

	return QualifiedName;
}

USoundWave* UAzSpeechHelper::ConvertWavFileToSoundWave(const FString& FilePath, const FString& FileName)
{
	if (AzSpeech::Internal::HasEmptyParam(FilePath, FileName))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: FilePath or FileName is empty"), *FString(__func__));
	}

	else if (const FString Full_FileName = QualifyWAVFileName(FilePath, FileName);
		FPlatformFileManager::Get().GetPlatformFile().FileExists(*Full_FileName))
	{
#if PLATFORM_ANDROID
		CheckAndroidPermission("android.permission.READ_EXTERNAL_STORAGE");
#endif

		if (TArray<uint8> RawData;
			FFileHelper::LoadFileToArray(RawData, *Full_FileName))
		{
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Result: Success"), *FString(__func__));
			return ConvertAudioDataToSoundWave(RawData);
		}
		// else
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Result: Failed to load file"), *FString(__func__));
	}

	UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Result: Cannot find the specified file"), *FString(__func__));
	return nullptr;
}

USoundWave* UAzSpeechHelper::ConvertAudioDataToSoundWave(const TArray<uint8>& RawData)
{
	if (!IsAudioDataValid(RawData))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: RawData is empty"), *FString(__func__));
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

		UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Result: Success"), *FString(__func__));
		return SoundWave;
	}

	UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Cannot create a new Sound Wave"), *FString(__func__));
	return nullptr;
}

FString UAzSpeechHelper::LoadXMLToString(const FString& FilePath, const FString& FileName)
{
	if (AzSpeech::Internal::HasEmptyParam(FilePath, FileName))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: FilePath or FileName is empty"), *FString(__func__));
	}
	else if (const FString Full_FileName = QualifyXMLFileName(FilePath, FileName);
		FPlatformFileManager::Get().GetPlatformFile().FileExists(*Full_FileName))
	{
		if (FString OutputStr;
			FFileHelper::LoadFileToString(OutputStr, *Full_FileName))
		{
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Result: %s loaded"), *FString(__func__), *Full_FileName);
			return OutputStr;
		}
	}

	UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Result: Failed to load file"), *FString(__func__));
	return FString();
}

bool UAzSpeechHelper::CreateNewDirectory(const FString& Path, const bool bCreateParents)
{
	bool bOutput = FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*Path);

	if (!bOutput)
	{
		UE_LOG(LogAzSpeech_Internal, Warning, TEXT("%s: Folder does not exist, trying to create a new with the specified path"), *FString(__func__));

		bOutput = bCreateParents
			          ? FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*Path)
			          : FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*Path);
	}

	if (bOutput)
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Result: Success. Output path: %s"), *FString(__func__), *Path);
	}
	else
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Result: Failed to create a new folder"), *FString(__func__));
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
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Result: Success"), *FString(__func__));
		}
		else
		{
			UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Result: Failed to open a folder picker or the user cancelled the operation"), *FString(__func__));
		}
	}
	else
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Result: Failed to get Desktop Platform"), *FString(__func__));
	}
#else
	UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Platform %s is not supported"), *FString(__func__), *UGameplayStatics::GetPlatformName());
#endif

	return OutputPath;
}

bool UAzSpeechHelper::CheckAndroidPermission([[maybe_unused]] const FString& InPermission)
{
#if PLATFORM_ANDROID
	UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Checking android permission: %s"), *FString(__func__), *InPermission);
	if (!UAndroidPermissionFunctionLibrary::CheckPermission(InPermission))
	{
		UAndroidPermissionFunctionLibrary::AcquirePermissions({ InPermission });
	}

	return UAndroidPermissionFunctionLibrary::CheckPermission(InPermission);
#else
	UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Platform %s is not supported"), *FString(__func__), *UGameplayStatics::GetPlatformName());
	return true;
#endif
}

bool UAzSpeechHelper::IsAudioDataValid(const TArray<uint8>& RawData)
{
	const bool bOutput = !AzSpeech::Internal::HasEmptyParam(RawData);
	if (!bOutput)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Invalid audio data."), *FString(__func__));
	}

	return bOutput;
}

int32 UAzSpeechHelper::CheckReturnFromRecognitionMap(const FString& InString, const FName GroupName, const bool bStopAtFirstTrigger)
{
	if (AzSpeech::Internal::HasEmptyParam(InString, GroupName))
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Invalid input string or group name"), *FString(__func__));
		return -1;
	}

	const TArray<FAzSpeechRecognitionData> DataArray = AzSpeech::Internal::GetRecognitionMap(GroupName);
	FAzSpeechRecognitionData OutputResult(-1);
	uint32 MatchPoints = 0u;

	const auto Comparisor_Lambda = [&InString, &GroupName, FuncName = __func__](const FString& KeyType, const FString& Key) -> bool
	{
		if (AzSpeech::Internal::HasEmptyParam(Key))
		{
			return false;
		}

		if (InString.Contains(Key, ESearchCase::IgnoreCase, ESearchDir::FromStart))
		{
			UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: String '%s' contains %s key '%s' from group %s"), *FString(FuncName), *InString, *KeyType, *Key, *GroupName.ToString());
			return true;
		}

		return false;
	};

	for (const FAzSpeechRecognitionData& Iterator : DataArray)
	{
		bool bIgnore = false;
		for (const FString& Ignore : Iterator.IgnoreKeys)
		{
			bIgnore = Comparisor_Lambda("ignore", Ignore);
		}

		if (bIgnore)
		{
			continue;
		}

		uint32 It_MatchPoints = 0u;
		for (const FString& Trigger : Iterator.TriggerKeys)
		{
			if (Comparisor_Lambda("trigger", Trigger))
			{
				++It_MatchPoints;

				if (bStopAtFirstTrigger)
				{
					UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Returning first triggered key from group %s. Result: %d"), *FString(__func__), *GroupName.ToString(), OutputResult.Value);
					return Iterator.Value;
				}
			}
		}

		if (It_MatchPoints > MatchPoints)
		{
			MatchPoints = It_MatchPoints;
			OutputResult = Iterator;
		}
	}

	if (OutputResult.Value < 0 || MatchPoints == 0u)
	{
		UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Failed to find matching data in recognition map group %s"), *FString(__func__), *GroupName.ToString());
	}
	else
	{
		UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Found matching data in recognition map group %s. Result: %d; Matching Points: %d"), *FString(__func__), *GroupName.ToString(), OutputResult.Value, MatchPoints);
	}

	return OutputResult.Value;
}
