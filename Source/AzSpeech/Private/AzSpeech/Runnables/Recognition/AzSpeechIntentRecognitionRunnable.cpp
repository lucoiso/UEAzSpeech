// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/Runnables/Recognition/AzSpeechIntentRecognitionRunnable.h"
#include "AzSpeech/AzSpeechSettings.h"
#include "AzSpeechInternalFuncs.h"
#include "LogAzSpeech.h"
#include <Async/Async.h>
#include <Misc/ScopeTryLock.h>

namespace MicrosoftSpeech = Microsoft::CognitiveServices::Speech;

uint32 FAzSpeechIntentRecognitionRunnable::Run()
{
    return FAzSpeechRunnableBase::Run();
}

void FAzSpeechIntentRecognitionRunnable::Exit()
{
    FAzSpeechRunnableBase::Exit();
}
