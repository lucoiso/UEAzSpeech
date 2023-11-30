// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechHelper.h"
#include "AzSpeechInternalFuncs.h"
#include "AzSpeech/Tasks/Recognition/KeywordRecognitionAsync.h"
#include "AzSpeech/Tasks/Recognition/SpeechToTextAsync.h"
#include "AzSpeech/Tasks/Recognition/WavFileToTextAsync.h"
#include "AzSpeech/Tasks/Synthesis/SSMLToAudioDataAsync.h"
#include "AzSpeech/Tasks/Synthesis/SSMLToSoundWaveAsync.h"
#include "AzSpeech/Tasks/Synthesis/SSMLToSpeechAsync.h"
#include "AzSpeech/Tasks/Synthesis/SSMLToWavFileAsync.h"
#include "AzSpeech/Tasks/Synthesis/TextToAudioDataAsync.h"
#include "AzSpeech/Tasks/Synthesis/TextToSoundWaveAsync.h"
#include "AzSpeech/Tasks/Synthesis/TextToSpeechAsync.h"
#include "AzSpeech/Tasks/Synthesis/TextToWavFileAsync.h"
#include <Audio.h>
#include <Sound/SoundWave.h>
#include <Misc/FileHelper.h>
#include <Misc/Paths.h>
#include <DesktopPlatformModule.h>
#include <Kismet/GameplayStatics.h>
#include <AudioCaptureCore.h>
#include <HAL/FileManager.h>
#include <AssetRegistry/AssetRegistryModule.h>
#include <Components/AudioComponent.h>
#include <AudioThread.h>
#include <AudioDeviceManager.h>
#include <Sound/AudioSettings.h>
#include <Engine/Engine.h>
#include <Interfaces/IPluginManager.h>
#include <Serialization/JsonReader.h>
#include <Serialization/JsonSerializer.h>
#include <Dom/JsonObject.h>
#include <Misc/PackageName.h>

#if WITH_EDITORONLY_DATA
#include <EditorFramework/AssetImportData.h>
#endif

#if ENGINE_MAJOR_VERSION >= 5
#include <UObject/SavePackage.h>
#endif

#if PLATFORM_ANDROID
#include <AndroidPermissionFunctionLibrary.h>
#endif

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AzSpeechHelper)
#endif

const FString UAzSpeechHelper::QualifyModulePath(const FString& ModuleName)
{
    FString Output = ModuleName;

    if (!Output.StartsWith("/"))
    {
        Output = "/" + Output;
    }
    if (!Output.EndsWith("/"))
    {
        Output += '/';
    }

    return Output;
}

const FString UAzSpeechHelper::QualifyPath(const FString& Path)
{
    FString Output = Path;
    FPaths::NormalizeDirectoryName(Output);

    if (!Output.EndsWith("/") && !Output.EndsWith("\""))
    {
        Output += '/';
    }

    UE_LOG(LogAzSpeech_Internal, Log, TEXT("%s: Qualified directory path: %s"), *FString(__func__), *Output);

    return Output;
}

const FString UAzSpeechHelper::QualifyFileExtension(const FString& Path, const FString& Name, const FString& Extension)
{
    if (AzSpeech::Internal::HasEmptyParam(Path, Name, Extension))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Filepath, Filename or Extension is empty"), *FString(__func__));
        return FString();
    }

    const FString LocalPath = QualifyPath(Path);
    const FString LocalExtension = Extension.Contains(".") ? Extension : "." + Extension;

    FString LocalName = Name;
    if (!Name.Right(Name.Len() - LocalExtension.Len()).Contains(LocalExtension))
    {
        LocalName += LocalExtension;
    }

    FString QualifiedName = LocalPath + LocalName;
    FPaths::NormalizeFilename(QualifiedName);

    UE_LOG(LogAzSpeech_Internal, Log, TEXT("%s: Qualified %s file path: %s"), *FString(__func__), *LocalExtension.ToUpper(), *QualifiedName);

    return QualifiedName;
}

USoundWave* UAzSpeechHelper::ConvertWavFileToSoundWave(const FString& FilePath, const FString& FileName, const FString& OutputModule, const FString& RelativeOutputDirectory, const FString& OutputAssetName)
{
    if (AzSpeech::Internal::HasEmptyParam(FilePath, FileName))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Filepath or Filename is empty"), *FString(__func__));
    }

    else if (const FString Full_FileName = QualifyWAVFileName(FilePath, FileName); IFileManager::Get().FileExists(*Full_FileName))
    {
#if PLATFORM_ANDROID
        if (!CheckAndroidPermission("android.permission.READ_EXTERNAL_STORAGE"))
        {
            return nullptr;
        }
#endif

        if (TArray<uint8> RawData; FFileHelper::LoadFileToArray(RawData, *Full_FileName))
        {
            UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Result: Success"), *FString(__func__));
            if (USoundWave* const SoundWave = ConvertAudioDataToSoundWave(RawData, OutputModule, RelativeOutputDirectory, OutputAssetName))
            {
#if WITH_EDITORONLY_DATA
                SoundWave->AssetImportData->Update(Full_FileName);
#endif
                return SoundWave;
            }
        }
        // else
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Result: Failed to load file '%s'"), *FString(__func__), *Full_FileName);
    }

    UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Result: Cannot find the specified file"), *FString(__func__));
    return nullptr;
}

USoundWave* UAzSpeechHelper::ConvertAudioDataToSoundWave(const TArray<uint8>& RawData, const FString& OutputModule, const FString& RelativeOutputDirectory, const FString& OutputAssetName)
{
#if PLATFORM_ANDROID
    if (!CheckAndroidPermission("android.permission.WRITE_EXTERNAL_STORAGE"))
    {
        return nullptr;
    }
#endif

    if (!IsAudioDataValid(RawData))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: RawData is empty"), *FString(__func__));
        return nullptr;
    }

    USoundWave* SoundWave = nullptr;
    TArray<UAudioComponent*> AudioComponentsToRestart;

    FWaveModInfo WaveInfo;
    WaveInfo.ReadWaveInfo(RawData.GetData(), RawData.Num());

    const int32 ChannelCount = static_cast<int32>(*WaveInfo.pChannels);
    const int32 SizeOfSample = (*WaveInfo.pBitsPerSample) / 8;
    const int32 NumSamples = WaveInfo.SampleDataSize / SizeOfSample;
    const int32 NumFrames = NumSamples / ChannelCount;

    bool bCreatedNewPackage = false;

    if (AzSpeech::Internal::HasEmptyParam(OutputModule) || AzSpeech::Internal::HasEmptyParam(OutputAssetName))
    {
        //Create a new object from the transient package
        SoundWave = NewObject<USoundWave>(GetTransientPackage(), *OutputAssetName);
    }
    else
    {
        if (!IsContentModuleAvailable(OutputModule))
        {
            UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Module '%s' is not available"), *FString(__func__), *OutputModule);
            return nullptr;
        }

        FString TargetFilename = FPaths::Combine(QualifyModulePath(OutputModule), RelativeOutputDirectory, OutputAssetName);
        FPaths::NormalizeFilename(TargetFilename);

        UPackage* const Package = CreatePackage(*TargetFilename);

        if (USoundWave* const ExistingSoundWave = FindObject<USoundWave>(Package, *OutputAssetName))
        {
            if (FAudioDeviceManager* const AudioDeviceManager = GEngine->GetAudioDeviceManager())
            {
                AudioDeviceManager->StopSoundsUsingResource(ExistingSoundWave, &AudioComponentsToRestart);
            }

            FAudioThread::RunCommandOnAudioThread([ExistingSoundWave]() { ExistingSoundWave->FreeResources(); });
            SoundWave = ExistingSoundWave;
        }
        else
        {
            SoundWave = NewObject<USoundWave>(Package, *OutputAssetName, RF_Public | RF_Standalone);
        }

        bCreatedNewPackage = true;
    }

    if (SoundWave)
    {
#if WITH_EDITORONLY_DATA
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
        SoundWave->RawData.UpdatePayload(FSharedBuffer::Clone(RawData.GetData(), RawData.Num()));
#else
        SoundWave->RawData.Lock(LOCK_READ_WRITE);
        void* LockedData = SoundWave->RawData.Realloc(RawData.Num());
        FMemory::Memcpy(LockedData, RawData.GetData(), RawData.Num());
        SoundWave->RawData.Unlock();
#endif
#endif

        SoundWave->RawPCMDataSize = WaveInfo.SampleDataSize;
        SoundWave->RawPCMData = static_cast<uint8*>(FMemory::Malloc(WaveInfo.SampleDataSize));
        FMemory::Memcpy(SoundWave->RawPCMData, WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);

        SoundWave->Duration = static_cast<float>(NumFrames) / *WaveInfo.pSamplesPerSec;
        SoundWave->SetSampleRate(*WaveInfo.pSamplesPerSec);
        SoundWave->NumChannels = ChannelCount;
        SoundWave->TotalSamples = *WaveInfo.pSamplesPerSec * SoundWave->Duration;

#if ENGINE_MAJOR_VERSION >= 5
        SoundWave->SetImportedSampleRate(*WaveInfo.pSamplesPerSec);

        SoundWave->CuePoints.Reset(WaveInfo.WaveCues.Num());
        for (FWaveCue& WaveCue : WaveInfo.WaveCues)
        {
            FSoundWaveCuePoint NewCuePoint;
            NewCuePoint.CuePointID = static_cast<int32>(WaveCue.CuePointID);
            NewCuePoint.FrameLength = static_cast<int32>(WaveCue.SampleLength);
            NewCuePoint.FramePosition = static_cast<int32>(WaveCue.Position);
            NewCuePoint.Label = WaveCue.Label;

            SoundWave->CuePoints.Add(NewCuePoint);
        }
#endif

#if WITH_EDITORONLY_DATA && (ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1))
        if (WaveInfo.TimecodeInfo.IsValid())
        {
            SoundWave->SetTimecodeInfo(*WaveInfo.TimecodeInfo);
        }
#endif

        if (bCreatedNewPackage)
        {
#if WITH_EDITORONLY_DATA && ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
            if (const UAudioSettings* const AudioSettings = GetDefault<UAudioSettings>())
            {
                FAudioThread::RunCommandOnAudioThread([AudioSettings, SoundWave]() { SoundWave->SetSoundAssetCompressionType(Audio::ToSoundAssetCompressionType(AudioSettings->DefaultAudioCompressionType)); });
            }
            else
            {
                FAudioThread::RunCommandOnAudioThread([SoundWave]() { SoundWave->SetSoundAssetCompressionType(ESoundAssetCompressionType::BinkAudio); });
            }

#elif ENGINE_MAJOR_VERSION == 5
            FAudioThread::RunCommandOnAudioThread([SoundWave]() { SoundWave->SetSoundAssetCompressionType(ESoundAssetCompressionType::BinkAudio); });
#endif

            SoundWave->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(SoundWave);

            const FString TempPackageName = SoundWave->GetPackage()->GetName();
            const FString TempPackageFilename = FPackageName::LongPackageNameToFilename(TempPackageName, FPackageName::GetAssetPackageExtension());

#if ENGINE_MAJOR_VERSION >= 5
            FSavePackageArgs SaveArgs;
            SaveArgs.SaveFlags = RF_Public | RF_Standalone;
            UPackage::SavePackage(SoundWave->GetPackage(), SoundWave, *TempPackageFilename, SaveArgs);
#else
            UPackage::SavePackage(SoundWave->GetPackage(), SoundWave, RF_Public | RF_Standalone, *TempPackageFilename);
#endif

#if WITH_EDITOR
            TArray<FAssetData> SyncAssets;
            SyncAssets.Add(FAssetData(SoundWave));
            GEditor->SyncBrowserToObjects(SyncAssets);
#endif
        }

        for (UAudioComponent* const& AudioComponent : AudioComponentsToRestart)
        {
            AudioComponent->Play();
        }

        UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Result: Success"), *FString(__func__));
        return SoundWave;
    }

    UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Cannot create a new Sound Wave"), *FString(__func__));
    return nullptr;
}

const FString UAzSpeechHelper::LoadXMLToString(const FString& FilePath, const FString& FileName)
{
    if (AzSpeech::Internal::HasEmptyParam(FilePath, FileName))
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: FilePath or FileName is empty"), *FString(__func__));
    }
    else if (const FString Full_FileName = QualifyXMLFileName(FilePath, FileName); IFileManager::Get().FileExists(*Full_FileName))
    {
        if (FString OutputStr; FFileHelper::LoadFileToString(OutputStr, *Full_FileName))
        {
            UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Result: '%s' loaded"), *FString(__func__), *Full_FileName);
            return OutputStr;
        }
    }

    UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Result: Failed to load file"), *FString(__func__));
    return FString();
}

const bool UAzSpeechHelper::CreateNewDirectory(const FString& Path, const bool bCreateParents)
{
    FString LocalPath = Path;
    FPaths::NormalizeDirectoryName(LocalPath);

    bool bOutput = FPaths::DirectoryExists(LocalPath);

    if (!bOutput)
    {
        UE_LOG(LogAzSpeech_Internal, Warning, TEXT("%s: Folder does not exist, trying to create a new with the specified path"), *FString(__func__));
        bOutput = IFileManager::Get().MakeDirectory(*LocalPath, bCreateParents);
    }

    if (bOutput)
    {
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Result: Success. Output path: %s"), *FString(__func__), *LocalPath);
    }
    else
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Result: Failed to create a new folder"), *FString(__func__));
    }

    return bOutput;
}

const FString UAzSpeechHelper::OpenDesktopFolderPicker()
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

const bool UAzSpeechHelper::CheckAndroidPermission([[maybe_unused]] const FString& InPermission)
{
#if PLATFORM_ANDROID
    UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Checking android permission: %s"), *FString(__func__), *InPermission);
    if (!UAndroidPermissionFunctionLibrary::CheckPermission(InPermission))
    {
        UAndroidPermissionFunctionLibrary::AcquirePermissions({ InPermission });
        return false;
    }

#else
    UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Platform %s is not supported"), *FString(__func__), *UGameplayStatics::GetPlatformName());
#endif

    return true;
}

const bool UAzSpeechHelper::IsAudioDataValid(const TArray<uint8>& RawData)
{
    const bool bOutput = !AzSpeech::Internal::HasEmptyParam(RawData);
    if (!bOutput)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Invalid audio data."), *FString(__func__));
    }

    return bOutput;
}

const TArray<FAzSpeechAudioInputDeviceInfo> UAzSpeechHelper::GetAvailableAudioInputDevices()
{
    TArray<FAzSpeechAudioInputDeviceInfo> Output;
    TArray<Audio::FCaptureDeviceInfo> Internal_Devices;

    if (Audio::FAudioCapture AudioCapture; AudioCapture.GetCaptureDevicesAvailable(Internal_Devices) <= 0)
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: There's no available audio input devices"), *FString(__func__));
    }
    else
    {
        UE_LOG(LogAzSpeech_Internal, Display, TEXT("%s: Result: Success"), *FString(__func__));

        for (const Audio::FCaptureDeviceInfo& DeviceInfo : Internal_Devices)
        {
            Output.Add(FAzSpeechAudioInputDeviceInfo(DeviceInfo.DeviceName, DeviceInfo.DeviceId));
            UE_LOG(LogAzSpeech_Debugging, Display, TEXT("%s: Found available audio input device: %s - %s"), *FString(__func__), *Output.Last().DeviceName, *Output.Last().GetAudioInputDeviceEndpointID());
        }
    }

    return Output;
}

template <typename ReturnTy>
const ReturnTy GetInformationFromDeviceID_T(const FString& DeviceID)
{
    const auto InvalidReturn_Lambda = []() -> ReturnTy
        {
            if constexpr (std::is_base_of<ReturnTy, bool>())
            {
                return false;
            }
            else if constexpr (std::is_base_of<ReturnTy, FAzSpeechAudioInputDeviceInfo>())
            {
                return FAzSpeechAudioInputDeviceInfo(FAzSpeechAudioInputDeviceInfo::InvalidDeviceID, FAzSpeechAudioInputDeviceInfo::InvalidDeviceID);
            }

            return ReturnTy();
        };

    if (!UAzSpeechHelper::IsAudioInputDeviceIDValid(DeviceID))
    {
        return InvalidReturn_Lambda();
    }

    for (const FAzSpeechAudioInputDeviceInfo& DeviceInfo : UAzSpeechHelper::GetAvailableAudioInputDevices())
    {
        if (DeviceInfo.GetDeviceID().Contains(DeviceID))
        {
            if constexpr (std::is_base_of<ReturnTy, bool>())
            {
                return true;
            }
            else if constexpr (std::is_base_of<ReturnTy, FAzSpeechAudioInputDeviceInfo>())
            {
                return DeviceInfo;
            }
        }
    }

    return InvalidReturn_Lambda();
}

const FAzSpeechAudioInputDeviceInfo UAzSpeechHelper::GetAudioInputDeviceInfoFromID(const FString& DeviceID)
{
    return GetInformationFromDeviceID_T<FAzSpeechAudioInputDeviceInfo>(DeviceID);
}

const bool UAzSpeechHelper::IsAudioInputDeviceAvailable(const FString& DeviceID)
{
    return GetInformationFromDeviceID_T<bool>(DeviceID);
}

const bool UAzSpeechHelper::IsAudioInputDeviceIDValid(const FString& DeviceID)
{
    return !(DeviceID.Contains(FAzSpeechAudioInputDeviceInfo::InvalidDeviceID) || DeviceID.Len() < std::strlen(FAzSpeechAudioInputDeviceInfo::PlaceholderDeviceID));
}

const TArray<FString> UAzSpeechHelper::GetAvailableContentModules()
{
    TArray<FString> Output{ "Game" };

    IPluginManager& PluginManager = IPluginManager::Get();
    const TArray<TSharedRef<IPlugin>> PluginsArray = PluginManager.GetEnabledPluginsWithContent();

    for (const TSharedRef<IPlugin>& Plugin : PluginsArray)
    {
        if (Plugin->GetLoadedFrom() != EPluginLoadedFrom::Project)
        {
            continue;
        }

        Output.Add(Plugin->GetName());
    }

    return Output;
}

const bool UAzSpeechHelper::IsContentModuleAvailable(const FString& ModuleName)
{
    const FString QualifiedParam = QualifyModulePath(ModuleName);

    bool bOutput = false;
    for (const FString& Module : GetAvailableContentModules())
    {
        if (QualifyModulePath(Module).Contains(QualifiedParam, ESearchCase::IgnoreCase))
        {
            bOutput = true;
            break;
        }
    }

    return bOutput;
}

const FString UAzSpeechHelper::GetPluginFriendlyName()
{
    return IPluginManager::Get().FindPlugin("AzSpeech")->GetFriendlyName();
}

const FString UAzSpeechHelper::GetPluginVersion()
{
    return IPluginManager::Get().FindPlugin("AzSpeech")->GetDescriptor().VersionName;
}

const FAzSpeechAnimationData UAzSpeechHelper::ExtractAnimationDataFromVisemeData(const FAzSpeechVisemeData& VisemeData)
{
    FAzSpeechAnimationData Output;
    if (AzSpeech::Internal::HasEmptyParam(VisemeData.Animation))
    {
        return Output;
    }

    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(VisemeData.Animation);
    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    FJsonSerializer::Deserialize(Reader, JsonObject);

    if (!JsonObject.IsValid())
    {
        UE_LOG(LogAzSpeech_Internal, Error, TEXT("%s: Failed to deserialize animation data"), *FString(__func__));
        return Output;
    }

    Output.FrameIndex = JsonObject->GetIntegerField("FrameIndex");

    for (const TSharedPtr<FJsonValue>& IteratorArray : JsonObject->GetArrayField("BlendShapes"))
    {
        FAzSpeechBlendShapes CurrentBlendShapes;
        for (const TSharedPtr<FJsonValue>& IteratorValue : IteratorArray->AsArray())
        {
            CurrentBlendShapes.Data.Add(static_cast<float>(IteratorValue->AsNumber()));
        }

        Output.BlendShapes.Add(CurrentBlendShapes);
    }

    return Output;
}

const TArray<FAzSpeechAnimationData> UAzSpeechHelper::ExtractAnimationDataFromVisemeDataArray(const TArray<FAzSpeechVisemeData>& VisemeData)
{
    TArray<FAzSpeechAnimationData> Output;

    for (const FAzSpeechVisemeData& VisemeDataElement : VisemeData)
    {
        Output.Add(ExtractAnimationDataFromVisemeData(VisemeDataElement));
    }

    return Output;
}

UAzSpeechTaskBase* UAzSpeechHelper::CastToAzSpeechTaskBase(UObject* const Object)
{
    return Cast<UAzSpeechTaskBase>(Object);
}

UAzSpeechRecognizerTaskBase* UAzSpeechHelper::CastToAzSpeechRecognizerTaskBase(UObject* const Object)
{
    return Cast<UAzSpeechRecognizerTaskBase>(Object);
}

UAzSpeechSynthesizerTaskBase* UAzSpeechHelper::CastToAzSpeechSynthesizerTaskBase(UObject* const Object)
{
    return Cast<UAzSpeechSynthesizerTaskBase>(Object);
}

const FString UAzSpeechHelper::GetAzSpeechLogsBaseDir()
{
    return FPaths::Combine(*FPaths::ProjectLogDir(), TEXT("AzSpeech"));
}

UAzSpeechTaskBase* UAzSpeechHelper::CreateKeywordRecognitionTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechRecognitionOptions& RecognitionOptions, const FString& AudioInputDeviceID, const FName& PhraseListGroup)
{
    return UKeywordRecognitionAsync::KeywordRecognition_CustomOptions(WorldContextObject, SubscriptionOptions, RecognitionOptions, AudioInputDeviceID, PhraseListGroup);
}

UAzSpeechTaskBase* UAzSpeechHelper::CreateSpeechToTextTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechRecognitionOptions& RecognitionOptions, const FString& AudioInputDeviceID, const FName& PhraseListGroup)
{
    return USpeechToTextAsync::SpeechToText_CustomOptions(WorldContextObject, SubscriptionOptions, RecognitionOptions, AudioInputDeviceID, PhraseListGroup);
}

UAzSpeechTaskBase* UAzSpeechHelper::CreateSSMLToAudioDataTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML)
{
    return USSMLToAudioDataAsync::SSMLToAudioData_CustomOptions(WorldContextObject, SubscriptionOptions, SynthesisOptions, SynthesisSSML);
}

UAzSpeechTaskBase* UAzSpeechHelper::CreateSSMLToSoundWaveTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML)
{
    return USSMLToSoundWaveAsync::SSMLToSoundWave_CustomOptions(WorldContextObject, SubscriptionOptions, SynthesisOptions, SynthesisSSML);
}

UAzSpeechTaskBase* UAzSpeechHelper::CreateSSMLToSpeechTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML)
{
    return USSMLToSpeechAsync::SSMLToSpeech_CustomOptions(WorldContextObject, SubscriptionOptions, SynthesisOptions, SynthesisSSML);
}

UAzSpeechTaskBase* UAzSpeechHelper::CreateSSMLToWavFileTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisSSML, const FString& FilePath, const FString& FileName)
{
    return USSMLToWavFileAsync::SSMLToWavFile_CustomOptions(WorldContextObject, SubscriptionOptions, SynthesisOptions, SynthesisSSML, FilePath, FileName);
}

UAzSpeechTaskBase* UAzSpeechHelper::CreateTextToAudioDataTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText)
{
    return UTextToAudioDataAsync::TextToAudioData_CustomOptions(WorldContextObject, SubscriptionOptions, SynthesisOptions, SynthesisText);
}

UAzSpeechTaskBase* UAzSpeechHelper::CreateTextToSoundWaveTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText)
{
    return UTextToSoundWaveAsync::TextToSoundWave_CustomOptions(WorldContextObject, SubscriptionOptions, SynthesisOptions, SynthesisText);
}

UAzSpeechTaskBase* UAzSpeechHelper::CreateTextToSpeechTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText)
{
    return UTextToSpeechAsync::TextToSpeech_CustomOptions(WorldContextObject, SubscriptionOptions, SynthesisOptions, SynthesisText);
}

UAzSpeechTaskBase* UAzSpeechHelper::CreateTextToWavFileTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechSynthesisOptions& SynthesisOptions, const FString& SynthesisText, const FString& FilePath, const FString& FileName)
{
    return UTextToWavFileAsync::TextToWavFile_CustomOptions(WorldContextObject, SubscriptionOptions, SynthesisOptions, SynthesisText, FilePath, FileName);
}

UAzSpeechTaskBase* UAzSpeechHelper::CreateWavFileToTextTask(UObject* const WorldContextObject, const FAzSpeechSubscriptionOptions& SubscriptionOptions, const FAzSpeechRecognitionOptions& RecognitionOptions, const FString& FilePath, const FString& FileName, const FName& PhraseListGroup)
{
    return UWavFileToTextAsync::WavFileToText_CustomOptions(WorldContextObject, SubscriptionOptions, RecognitionOptions, FilePath, FileName, PhraseListGroup);
}