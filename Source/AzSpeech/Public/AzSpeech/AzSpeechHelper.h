// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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
	static FString QualifyPath(const FString& Path);

	/* Helper function to qualify the extension of a given file */
	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (DisplayName = "Qualify File Extension"))
	static FString QualifyFileExtension(const FString& Path, const FString& Name, const FString& Extension);

	/* Helper function to qualify a WAV file path + name to a single string like Full/File/Path/Filename.wav */
	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (DisplayName = "Qualify WAV File Path"))
	static FString QualifyWAVFileName(const FString& Path, const FString& Name)
	{
		return QualifyFileExtension(Path, Name, "wav");
	}

	/* Helper function to qualify a XML file path + name to a single string like Full/File/Path/Filename.xml */
	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (DisplayName = "Qualify XML File Path"))
	static FString QualifyXMLFileName(const FString& Path, const FString& Name)
	{
		return QualifyFileExtension(Path, Name, "xml");
	}

	/* Convert .wav file to USoundWave */
	UFUNCTION(BlueprintPure, Category = "AzSpeech", Meta = (DisplayName = "Convert .wav file to USoundWave"))
	static USoundWave* ConvertWavFileToSoundWave(const FString& FilePath, const FString& FileName);

	/* Convert audio data (TArray<uint8>) to USoundWave */
	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static USoundWave* ConvertAudioDataToSoundWave(const TArray<uint8>& RawData);

	/* Load a given .xml file and return the content as string */
	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (DisplayName = "Load XML to String"))
	static FString LoadXMLToString(const FString& FilePath, const FString& FileName);

	/* Create a new directory in the specified location */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static bool CreateNewDirectory(const FString& Path, const bool bCreateParents = true);

	/* Opens the desktop folder picker and return the selected folder path as string */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech")
	static FString OpenDesktopFolderPicker();

	/* Check if the android platform already has permission and add if not */
	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (DisplayName = "Check and Add Android Permission"))
	static bool CheckAndroidPermission(const FString& InPermission);

	/* Check if the audio data is valid or not */
	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static bool IsAudioDataValid(const TArray<uint8>& RawData);

	/* Search in the recognition map for the key that best matches with the input string and return the registered value (See Project Settings -> AzSpeech: Recognition Map) */
	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	static int32 CheckReturnFromRecognitionMap(const FString& InString, const FName GroupName, const bool bStopAtFirstTrigger = false);

};
