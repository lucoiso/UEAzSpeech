// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechHelper.h"
#include "Sound/SoundWave.h"
#include "Misc/FileHelper.h"

USoundWave* CreateNewSoundWave(const FString ObjectName)
{
	USoundWave* SoundWave = FindObject<USoundWave>(ANY_PACKAGE, *ObjectName);

	if (!IsValid(SoundWave))
	{
		SoundWave = NewObject<USoundWave>((UObject*)GetTransientPackage(), *ObjectName, RF_Transient);
	}
	else
	{
		if (SoundWave->RawData.IsLocked())
		{
			SoundWave->RawData.Unlock();
		}

		SoundWave->RawData.RemoveBulkData();
	}

	return SoundWave;
}

USoundWave* UAzSpeechHelper::ConvertFileIntoSoundWave(const FString FilePath, const FString ObjectName)
{
	TArray<uint8> RawData;
	if (FFileHelper::LoadFileToArray(RawData, *FilePath, EFileRead::FILEREAD_NoFail))
	{
		return ConvertStreamIntoSoundWave(RawData, ObjectName);
	}

	return nullptr;
}

USoundWave* UAzSpeechHelper::ConvertStreamIntoSoundWave(TArray<uint8> RawData, const FString ObjectName)
{
	if (!RawData.IsEmpty())
	{
		USoundWave* SoundWave = CreateNewSoundWave(ObjectName);

		SoundWave->RawData.Lock(LOCK_READ_WRITE);
		void* RawDataPtr = SoundWave->RawData.Realloc(RawData.Num());
		FMemory::Memcpy(RawDataPtr, RawData.GetData(), RawData.Num());
		SoundWave->RawData.Unlock();

		FWaveModInfo WaveInfo;
		WaveInfo.ReadWaveInfo(RawData.GetData(), RawData.Num());

		const int32 ChannelCount = *WaveInfo.pChannels;
		const int32 SizeOfSample = *WaveInfo.pBitsPerSample / 8;
		const int32 NumSamples = WaveInfo.SampleDataSize / SizeOfSample;
		const int32 NumFrames = NumSamples / ChannelCount;

		SoundWave->Duration = NumFrames / (*WaveInfo.pSamplesPerSec);
		SoundWave->SetImportedSampleRate(*WaveInfo.pSamplesPerSec);
		SoundWave->SetSampleRate(*WaveInfo.pSamplesPerSec);
		SoundWave->NumChannels = ChannelCount;
		SoundWave->TotalSamples = *WaveInfo.pSamplesPerSec * SoundWave->Duration;

		SoundWave->InitAudioResource(SoundWave->RawData);

		return SoundWave;
	}

	return nullptr;
}