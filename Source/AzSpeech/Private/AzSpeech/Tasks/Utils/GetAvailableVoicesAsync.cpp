// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Tasks/Utils/GetAvailableVoicesAsync.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>

THIRD_PARTY_INCLUDES_START
#include <speechapi_cxx_speech_synthesizer.h>
THIRD_PARTY_INCLUDES_END

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(GetAvailableVoicesAsync)
#endif

namespace MicrosoftSpeech = Microsoft::CognitiveServices::Speech;

UGetAvailableVoicesAsync* UGetAvailableVoicesAsync::GetAvailableVoicesAsync(UObject* const WorldContextObject, const FString& Locale)
{
    UGetAvailableVoicesAsync* const NewAsyncTask = NewObject<UGetAvailableVoicesAsync>();
    NewAsyncTask->TaskName = *FString(__func__);
    NewAsyncTask->Locale = Locale;

    NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

    return NewAsyncTask;
}

void UGetAvailableVoicesAsync::Activate()
{
    if (!UAzSpeechSettings::CheckAzSpeechSettings())
    {
        SetReadyToDestroy();
        return;
    }

    Super::Activate();

    UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Activating task"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
        [this]
        {
            const TArray<FString> TaskResult = GetAvailableVoices();
            AsyncTask(ENamedThreads::GameThread,
                [this, TaskResult]
                {
                    BroadcastResult(TaskResult);
                }
            );
        }
    );
}

void UGetAvailableVoicesAsync::SetReadyToDestroy()
{
    UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Setting task as Ready to Destroy"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));

    Super::SetReadyToDestroy();
}

void UGetAvailableVoicesAsync::BroadcastResult(const TArray<FString>& Result)
{
    check(IsInGameThread());

    if (Result.Num() <= 0)
    {
        UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Task failed. Broadcasting failure"), *TaskName.ToString(), GetUniqueID(), *FString(__func__));
        Fail.Broadcast();
    }
    else
    {
        UE_LOG(LogAzSpeech, Display, TEXT("Task: %s (%d); Function: %s; Message: Task completed. Broadcasting result with size: %d"), *TaskName.ToString(), GetUniqueID(), *FString(__func__), Result.Num());
        Success.Broadcast(Result);
    }

    if (Fail.IsBound())
    {
        Fail.Clear();
    }

    if (Success.IsBound())
    {
        Success.Clear();
    }

    SetReadyToDestroy();
}

const TArray<FString> UGetAvailableVoicesAsync::GetAvailableVoices() const
{
    TArray<FString> Output;

    std::shared_ptr<MicrosoftSpeech::SpeechConfig> SpeechConfig;
    const UAzSpeechSettings* const Settings = UAzSpeechSettings::Get();
    const std::string SubscriptionKey = TCHAR_TO_UTF8(*Settings->DefaultOptions.SubscriptionOptions.SubscriptionKey.ToString());

    if (Settings->DefaultOptions.SubscriptionOptions.bUsePrivateEndpoint)
    {
        const std::string RegionID = TCHAR_TO_UTF8(*Settings->DefaultOptions.SubscriptionOptions.RegionID.ToString());
        SpeechConfig = MicrosoftSpeech::SpeechConfig::FromSubscription(SubscriptionKey, RegionID);
    }
    else
    {
        const std::string Endpoint = TCHAR_TO_UTF8(*Settings->DefaultOptions.SubscriptionOptions.PrivateEndpoint.ToString());
        SpeechConfig = MicrosoftSpeech::SpeechConfig::FromEndpoint(Endpoint, SubscriptionKey);
    }

    if (SpeechConfig)
    {
        if (std::shared_ptr<MicrosoftSpeech::SpeechSynthesizer> SpeechSynthesizer = MicrosoftSpeech::SpeechSynthesizer::FromConfig(SpeechConfig))
        {
            const auto SynthesisVoices = SpeechSynthesizer->GetVoicesAsync(TCHAR_TO_UTF8(*Locale)).get();

            for (const auto& Voice : SynthesisVoices->Voices)
            {
                Output.Emplace(UTF8_TO_TCHAR(Voice->ShortName.c_str()));

                const FStringFormatOrderedArguments Arguments{
                    TaskName.ToString(),
                    GetUniqueID(),
                    FString(__func__),
                    FString(UTF8_TO_TCHAR(Voice->Name.c_str())),
                    FString(UTF8_TO_TCHAR(Voice->ShortName.c_str())),
                    FString(UTF8_TO_TCHAR(Voice->LocalName.c_str())),
                    FString(UTF8_TO_TCHAR(Voice->VoicePath.c_str())),
                    FString(UTF8_TO_TCHAR(Voice->Locale.c_str())),
                    static_cast<int32>(Voice->Gender),
                    static_cast<int32>(Voice->VoiceType)
                };

                const FString MountedDebuggingInfo = FString::Format(TEXT("Task: {0} ({1});\n\tFunction: {2};\n\tVoice Name: {3}\n\tVoice Short Name: {4}\n\tVoice Local Name: {5}\n\tVoice Path: {6};\n\tVoice Locale: {7}\n\tVoice Gender: {8}\n\tVoice Type: {9}"), Arguments);

                UE_LOG(LogAzSpeech_Debugging, Display, TEXT("%s"), *MountedDebuggingInfo);

#if !UE_BUILD_SHIPPING
                if (UAzSpeechSettings::Get()->bEnableDebuggingPrints)
                {
                    GEngine->AddOnScreenDebugMessage(static_cast<int32>(GetUniqueID()), 5.f, FColor::Yellow, MountedDebuggingInfo);
                }
#endif
            }
        }
    }

    return Output;
}