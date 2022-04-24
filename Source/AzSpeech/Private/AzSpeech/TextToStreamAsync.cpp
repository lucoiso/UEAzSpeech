// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/TextToStreamAsync.h"
#include "AzSpeechWrapper.h"

UTextToStreamAsync* UTextToStreamAsync::TextToStreamAsync(const UObject* WorldContextObject,
                                                          const FString TextToConvert, const FString VoiceName,
                                                          const FAzSpeechData Parameters)
{
	UTextToStreamAsync* NewAsyncTask = NewObject<UTextToStreamAsync>();
	NewAsyncTask->WorldContextObject = WorldContextObject;
	NewAsyncTask->TextToConvert = TextToConvert;
	NewAsyncTask->VoiceName = VoiceName;
	NewAsyncTask->Parameters = Parameters;

	return NewAsyncTask;
}

void UTextToStreamAsync::Activate()
{
	FAzSpeechWrapper::Unreal_Cpp::AsyncTextToStream(TextToConvert, VoiceName, Parameters, TaskCompleted);
}
