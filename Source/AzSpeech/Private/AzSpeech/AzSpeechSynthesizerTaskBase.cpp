// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechSynthesizerTaskBase.h"
#include "AzSpeechInternalFuncs.h"

void UAzSpeechSynthesizerTaskBase::Activate()
{
	Super::Activate();
}

void UAzSpeechSynthesizerTaskBase::StopAzSpeechTask()
{
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

void UAzSpeechSynthesizerTaskBase::StartAzureTaskWork_Internal()
{
	Super::StartAzureTaskWork_Internal();
}
