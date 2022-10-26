// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeechInternalFuncs.h"
#include "Async/Async.h"

void UAzSpeechSynthesizerTaskBase::Activate()
{
	Super::Activate();
}

void UAzSpeechSynthesizerTaskBase::StopAzSpeechTask()
{
	UE_LOG(LogAzSpeech, Display, TEXT("%s - Finishing AzSpeech Task"), *FString(__func__)); 
	
	if (SynthesizerObject)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&]
		{
			const TFuture<void> StopTaskWork = Async(EAsyncExecution::Thread, [=]() -> void
			{
				return SynthesizerObject->StopSpeakingAsync().get();
			});

			StopTaskWork.WaitFor(FTimespan::FromSeconds(AzSpeech::Internal::GetTimeout()));
			SynthesizerObject.reset();

			AsyncTask(ENamedThreads::GameThread, [this] { SetReadyToDestroy(); });
		});
	}
}

bool UAzSpeechSynthesizerTaskBase::StartAzureTaskWork_Internal()
{
	return Super::StartAzureTaskWork_Internal();
}
