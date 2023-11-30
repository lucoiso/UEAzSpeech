// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "AzSpeech/Structures/AzSpeechAudioInputDeviceInfo.h"
#include "AzSpeech/Structures/AzSpeechAnimationData.h"
#include "AzSpeech/Structures/AzSpeechVisemeData.h"
#include "AzSpeechHelper.generated.h"

/**
 *
 */
UCLASS(NotPlaceable, Category = "AzSpeech")
class AZSPEECH_API UAzSpeechHelper final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /* Helper function to qualify a Module Name string to a single string like /ModulePath/ */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Utils", meta = (DisplayName = "Qualify Module Path"))
    static const FString QualifyModulePath(const FString& ModuleName);

    /* Helper function to qualify a path string to a single string like Full/File/Path/ */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Utils", meta = (DisplayName = "Qualify Path"))
    static const FString QualifyPath(const FString& Path);

    /* Helper function to qualify the extension of a given file */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Utils", meta = (DisplayName = "Qualify File Extension"))
    static const FString QualifyFileExtension(const FString& Path, const FString& Name, const FString& Extension);

    /* Helper function to qualify a WAV file path + name to a single string like Full/File/Path/Filename.wav */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Utils", meta = (DisplayName = "Qualify WAV File Path"))
    static const FString QualifyWAVFileName(const FString& Path, const FString& Name)
    {
        return QualifyFileExtension(Path, Name, "wav");
    }

    /* Helper function to qualify a XML file path + name to a single string like Full/File/Path/Filename.xml */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Utils", meta = (DisplayName = "Qualify XML File Path"))
    static const FString QualifyXMLFileName(const FString& Path, const FString& Name)
    {
        return QualifyFileExtension(Path, Name, "xml");
    }

    /*
        Convert .wav file to USoundWave.

        [OutputModule, RelativeOutputDirectory, OutputAssetName]: Used to save the generated audio data in project's content. Set empty values to generate a transient Sound Wave

        OutputModule: Name of the module that will be used to save the generated audio data in project's content. Example: Game.
        RelativeOutputDirectory: Directory where the sound wave will be saved
        OutputAssetName: Name of the generated Sound Wave

        Use GetAvailableContentModules or look at the Audio Generator tool to check available modules
    */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Audio", Meta = (DisplayName = "Convert .wav file to USoundWave"))
    static USoundWave* ConvertWavFileToSoundWave(const FString& FilePath, const FString& FileName, const FString& OutputModule = "", const FString& RelativeOutputDirectory = "", const FString& OutputAssetName = "");

    /*
        Convert audio data (TArray<uint8>) to USoundWave.

        [OutputModule, RelativeOutputDirectory, OutputAssetName]: Used to save the generated audio data in project's content. Set empty values to generate a transient Sound Wave

        OutputModule: Name of the module that will be used to save the generated audio data in project's content. Example: Game.
        RelativeOutputDirectory: Directory where the sound wave will be saved
        OutputAssetName: Name of the generated Sound Wave

        Use GetAvailableContentModules or look at the Audio Generator tool to check available modules
    */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Audio")
    static USoundWave* ConvertAudioDataToSoundWave(const TArray<uint8>& RawData, const FString& OutputModule = "", const FString& RelativeOutputDirectory = "", const FString& OutputAssetName = "");

    /* Load a given .xml file and return the content as string */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Utils", meta = (DisplayName = "Load XML to String"))
    static const FString LoadXMLToString(const FString& FilePath, const FString& FileName);

    /* Create a new directory in the specified location */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Utils")
    static const bool CreateNewDirectory(const FString& Path, const bool bCreateParents = true);

    /* Opens the desktop folder picker and return the selected folder path as string */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Utils")
    static const FString OpenDesktopFolderPicker();

    /* Check if the android platform already has permission and add if not */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Utils", meta = (DisplayName = "Check and Add Android Permission"))
    static const bool CheckAndroidPermission(const FString& InPermission);

    /* Check if the audio data is valid or not */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Audio")
    static const bool IsAudioDataValid(const TArray<uint8>& RawData);

    /* Get the available audio input devices in the current platform */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Audio")
    static const TArray<FAzSpeechAudioInputDeviceInfo> GetAvailableAudioInputDevices();

    /* Get the audio input devices info by it's ID */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Audio")
    static const FAzSpeechAudioInputDeviceInfo GetAudioInputDeviceInfoFromID(const FString& DeviceID);

    /* Check if the audio input device is currently available */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Audio")
    static const bool IsAudioInputDeviceAvailable(const FString& DeviceID);

    /* Check if the device ID is valid */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Audio", Meta = (DisplayName = "Is Audio Input Device ID Valid"))
    static const bool IsAudioInputDeviceIDValid(const FString& DeviceID);

    /* Get available modules with content enabled */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Utils")
    static const TArray<FString> GetAvailableContentModules();

    /* Check if the content module is available */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Utils")
    static const bool IsContentModuleAvailable(const FString& ModuleName);

    /* Get AzSpeech Friendly Name */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Utils")
    static const FString GetPluginFriendlyName();

    /* Get AzSpeech Version */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Utils")
    static const FString GetPluginVersion();

    /*
        Extract the Animation JSON property from Viseme Data.

        JSON Body Format:
        [
            FrameIndex: Integer,
            BlendShapes: [
                [
                    Number,
                    ...
                ],
                [
                    Number,
                    ...
                ],
                ...
            ]
        ]
    */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Data")
    static const FAzSpeechAnimationData ExtractAnimationDataFromVisemeData(const FAzSpeechVisemeData& VisemeData);

    /*
        Extract the Animation JSON property from Viseme Data Array.

        JSON Body Format:
        [
            FrameIndex: Integer,
            BlendShapes: [
                [
                    Number,
                    ...
                ],
                [
                    Number,
                    ...
                ],
                ...
            ]
        ]
    */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Data")
    static const TArray<FAzSpeechAnimationData> ExtractAnimationDataFromVisemeDataArray(const TArray<FAzSpeechVisemeData>& VisemeData);

    /* Cast object to AzSpeech Task Base */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Casting")
    static class UAzSpeechTaskBase* CastToAzSpeechTaskBase(UObject* const Object);

    /* Cast object to AzSpeech Recognizer Task Base */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Casting")
    static class UAzSpeechRecognizerTaskBase* CastToAzSpeechRecognizerTaskBase(UObject* const Object);

    /* Cast object to AzSpeech Synthesizer Task Base */
    UFUNCTION(BlueprintPure, Category = "AzSpeech | Casting")
    static class UAzSpeechSynthesizerTaskBase* CastToAzSpeechSynthesizerTaskBase(UObject* const Object);

    static const FString GetAzSpeechLogsBaseDir();

    /* Create a task object that doesnt activate on creation. Use it to insert the task in an execution queue of AzSpeech Subsystem */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Execution Queue", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "PhraseListGroup"))
    static class UAzSpeechTaskBase* CreateKeywordRecognitionTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechRecognitionOptions& RecognitionOptions, const FString& AudioInputDeviceID = "Default", const FName& PhraseListGroup = NAME_None);

    /* Create a task object that doesnt activate on creation. Use it to insert the task in an execution queue of AzSpeech Subsystem */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Execution Queue", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "PhraseListGroup"))
    static class UAzSpeechTaskBase* CreateSpeechToTextTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechRecognitionOptions& RecognitionOptions, const FString& AudioInputDeviceID = "Default", const FName& PhraseListGroup = NAME_None);

    /* Create a task object that doesnt activate on creation. Use it to insert the task in an execution queue of AzSpeech Subsystem */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Execution Queue", meta = (WorldContext = "WorldContextObject"))
    static class UAzSpeechTaskBase* CreateSSMLToAudioDataTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML);

    /* Create a task object that doesnt activate on creation. Use it to insert the task in an execution queue of AzSpeech Subsystem */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Execution Queue", meta = (WorldContext = "WorldContextObject"))
    static class UAzSpeechTaskBase* CreateSSMLToSoundWaveTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML);

    /* Create a task object that doesnt activate on creation. Use it to insert the task in an execution queue of AzSpeech Subsystem */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Execution Queue", meta = (WorldContext = "WorldContextObject"))
    static class UAzSpeechTaskBase* CreateSSMLToSpeechTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML);

    /* Create a task object that doesnt activate on creation. Use it to insert the task in an execution queue of AzSpeech Subsystem */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Execution Queue", meta = (WorldContext = "WorldContextObject"))
    static class UAzSpeechTaskBase* CreateSSMLToWavFileTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML, const FString& FilePath, const FString& FileName);

    /* Create a task object that doesnt activate on creation. Use it to insert the task in an execution queue of AzSpeech Subsystem */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Execution Queue", meta = (WorldContext = "WorldContextObject"))
    static class UAzSpeechTaskBase* CreateTextToAudioDataTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText);

    /* Create a task object that doesnt activate on creation. Use it to insert the task in an execution queue of AzSpeech Subsystem */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Execution Queue", meta = (WorldContext = "WorldContextObject"))
    static class UAzSpeechTaskBase* CreateTextToSoundWaveTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText);

    /* Create a task object that doesnt activate on creation. Use it to insert the task in an execution queue of AzSpeech Subsystem */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Execution Queue", meta = (WorldContext = "WorldContextObject"))
    static class UAzSpeechTaskBase* CreateTextToSpeechTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText);

    /* Create a task object that doesnt activate on creation. Use it to insert the task in an execution queue of AzSpeech Subsystem */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Execution Queue", meta = (WorldContext = "WorldContextObject"))
    static class UAzSpeechTaskBase* CreateTextToWavFileTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText, const FString& FilePath, const FString& FileName);

    /* Create a task object that doesnt activate on creation. Use it to insert the task in an execution queue of AzSpeech Subsystem */
    UFUNCTION(BlueprintCallable, Category = "AzSpeech | Execution Queue", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "PhraseListGroup"))
    static class UAzSpeechTaskBase* CreateWavFileToTextTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechRecognitionOptions& RecognitionOptions, const FString& FilePath, const FString& FileName, const FName& PhraseListGroup = NAME_None);
};
