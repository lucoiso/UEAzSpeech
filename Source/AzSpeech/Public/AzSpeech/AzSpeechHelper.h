// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "AzSpeech/AzSpeechAudioInputDeviceInfo.h"
#include "AzSpeechHelper.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UAzSpeechHelper final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* Helper function to qualify a path string to a single string like Full/File/Path/ */
	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (DisplayName = "Qualify Path"))
	static const FString QualifyPath(const FString& Path);

	/* Helper function to qualify the extension of a given file */
	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (DisplayName = "Qualify File Extension"))
	static const FString QualifyFileExtension(const FString& Path, const FString& Name, const FString& Extension);

	/* Helper function to qualify a WAV file path + name to a single string like Full/File/Path/Filename.wav */
	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (DisplayName = "Qualify WAV File Path"))
	static const FString QualifyWAVFileName(const FString& Path, const FString& Name)
	{
		return QualifyFileExtension(Path, Name, "wav");
	}

	/* Helper function to qualify a XML file path + name to a single string like Full/File/Path/Filename.xml */
	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (DisplayName = "Qualify XML File Path"))
	static const FString QualifyXMLFileName(const FString& Path, const FString& Name)
	{
		return QualifyFileExtension(Path, Name, "xml");
	}

	/* Convert .wav file to USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", Meta = (DisplayName = "Convert .wav file to USoundWave"))
	static USoundWave* ConvertWavFileToSoundWave(const FString& FilePath, const FString& FileName);

	/* Convert audio data (TArray<uint8>) to USoundWave */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static USoundWave* ConvertAudioDataToSoundWave(const TArray<uint8>& RawData);

	/* Load a given .xml file and return the content as string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (DisplayName = "Load XML to String"))
	static const FString LoadXMLToString(const FString& FilePath, const FString& FileName);

	/* Create a new directory in the specified location */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static const bool CreateNewDirectory(const FString& Path, const bool bCreateParents = true);

	/* Opens the desktop folder picker and return the selected folder path as string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static const FString OpenDesktopFolderPicker();

	/* Check if the android platform already has permission and add if not */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (DisplayName = "Check and Add Android Permission"))
	static const bool CheckAndroidPermission(const FString& InPermission);

	/* Check if the audio data is valid or not */
	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static const bool IsAudioDataValid(const TArray<uint8>& RawData);

	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static const TArray<FAzSpeechAudioInputDeviceInfo> GetAvailableAudioInputDevices();
	
	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static const FAzSpeechAudioInputDeviceInfo GetAudioInputDeviceInfoFromID(const FString& DeviceID);

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static const bool IsAudioInputDeviceAvailable(const FString& DeviceID);

	UFUNCTION(BlueprintPure, Category = "AzSpeech", Meta = (DisplayName = "Is Audio Input Device ID Valid"))
	static const bool IsAudioInputDeviceIDValid(const FString& DeviceID);
};
