# Plugin: AzSpeech
## About the project:

A plugin integrating Azure Speech Cognitive Services to Unreal Engine with simple functions which can do these asynchronous tasks: 
  1. Voice-To-Text (Convert a speech into a string)
  2. Text-To-Voice (Convert a string into a speech)
  3. Text-To-Wav (Convert a string into a .wav audio file)

**Product on Unreal Engine Marketplace:** https://www.unrealengine.com/marketplace/en-US/product/azspeech-async-text-to-voice-and-voice-to-text  
**Post on Unreal Engine Forum:** https://forums.unrealengine.com/t/free-azspeech-plugin-async-text-to-voice-and-voice-to-text-with-microsoft-azure/495394  

# Documentation
## Installation
Just clone this repository or download the .zip file in the latest version and drop the files inside the 'Plugins/' folder of your project folder.
Note that if your project doesn't have a 'Plugins' folder, you can create one.

## Blueprint Usage:

![image](https://user-images.githubusercontent.com/77353979/157914741-628ab8e4-6882-47e6-bede-eee2f72ecbec.png)  
You have these asynchronous functions to manage all the workaround:

> ![image](https://user-images.githubusercontent.com/77353979/157915810-15434664-5681-4538-89c2-801df6934749.png)  
>  1. **Voice to Text Async**: Will convert your speech to string. Not that this function will get the speech by your default sound input device.  
>  1.1. **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  


> ![image](https://user-images.githubusercontent.com/77353979/157915746-b6681afa-6bce-447e-acc9-1bcb6824db51.png)  
>  2. **Text to Voice Async**: Will convert the specified text to a sound that will be played by the default sound output device.  
>  2.1. **Text to Convert**: The text that will be converted to audio;  
>  2.2. **Voice Name**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
>  2.3. **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  


> ![image](https://user-images.githubusercontent.com/77353979/157916050-6a691103-d143-4296-a561-21471dbf9e57.png)  
>  3. **Text to WAV Async**: Will convert the specified text to a .wav file.  
>  3.1.  **Text to Convert**: The text that will be converted to audio;  
>  3.2. **Voice Name**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
>  3.3. **File Path**: Output path to save the audio file;  
>  3.4. **File Name**: Output file name;  
>  3.5. **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  


>![image](https://user-images.githubusercontent.com/77353979/157916110-9e6a89f6-da94-46ef-bc78-287830bc3e7f.png)  
> 4. **AzSpeechData**: Represents Microsoft Azure parameters to connect to the service and perform the tasks;  
> 4.1. **API Access Key**: It's your Speech Service API Access Key from your Microsoft Azure Portal - Speech Service Panel;  
> 4.2. **Region ID**: Speech Service Region from your Microsoft Azure Portal. You can see all regions here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions;  
> 4.3. **Language ID**: Language to apply lozalization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
 
## C++ Usage

**You need to include the module "AzSpeech" inside your .Build.cs class to be allowed to call AzSpeech functions from C++.**
![image](https://user-images.githubusercontent.com/77353979/157924008-6ce0c137-3c21-4c82-b0b9-de1a0ab3ac9f.png)

> ## Text-to-Voice:
>``` 
> #include "AzSpeech/TextToVoiceAsync.h"
> 
> void AMyExampleActor::Example()
> {
>   FAzSpeechData MyNewData;
>   MyNewData.LanguageID = "LanguageID";
>   MyNewData.RegionID = "RegionID";
>   MyNewData.APIAccessKey = "APIAccessKey";
>
>   UTextToVoiceAsync* MyTask = UTextToVoiceAsync::TextToVoiceAsync(this, "Text to Convert",
>		                                                                "Voice Name", MyNewData);
>
>   MyTask->TaskCompleted.AddDynamic(this, &AMyExampleActor::ExampleCallback);
>
>   MyTask->Activate();
> }
>
> void AMyExampleActor::ExampleCallback(const bool TaskResult)
> {
> }
>```
> 1. **UTextToVoiceAsync::TextToVoiceAsync(WorldContextObject, TextToConvert, VoiceName, FAzSpeechData)**: Will convert the specified text to a sound that will be played by the default sound output device.  
> 1.1. **WorldContextObject**: Task Owner.  
> 1.2. **TextToConvert**: The text that will be converted to audio;  
> 1.3. **VoiceName**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
> 1.4. **FAzSpeechData**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  
>
> 2. **FAzSpeechData**: Represents Microsoft Azure parameters to connect to the service and perform the tasks;  
> 2.1. **APIAccessKey**: It's your Speech Service API Access Key from your Microsoft Azure Portal - Speech Service Panel;  
> 2.2. **RegionID**: Speech Service Region from your Microsoft Azure Portal. You can see all regions here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions;  
> 2.3. **LanguageID**: Language to apply lozalization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
>
> The **TextToVoiceAsync** function return a delegate to manage task status which allow you to bind a function to it's delegate to handle task's completion call.

> ## Voice-to-Text:
>```
> #include "AzSpeech/VoiceToTextAsync.h"
> 
> void AMyExampleActor::Example()
> {
>   FAzSpeechData MyNewData;
>   MyNewData.LanguageID = "LanguageID";
>   MyNewData.RegionID = "RegionID";
>   MyNewData.APIAccessKey = "APIAccessKey";
>
>   UVoiceToTextAsync* MyTask = UVoiceToTextAsync::VoiceToTextAsync(this, MyNewData);
>
>   MyTask->TaskCompleted.AddDynamic(this, &AMyExampleActor::ExampleCallback);
>
>   MyTask->Activate();
> }
>
> void AMyExampleActor::ExampleCallback(const FString& TaskResult)
> {
> }
>```
>
> 1. **UVoiceToTextAsync::VoiceToTextAsync(WorldContextObject, FAzSpeechData)**: Will convert the specified text to a sound that will be played by the default sound output device.  
> 1.1. **WorldContextObject**: Task Owner.  
> 1.2. **FAzSpeechData**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  
> 
> 2. **FAzSpeechData**: Represents Microsoft Azure parameters to connect to the service and perform the tasks;  
> 2.1. **APIAccessKey**: It's your Speech Service API Access Key from your Microsoft Azure Portal - Speech Service Panel;  
> 2.2. **RegionID**: Speech Service Region from your Microsoft Azure Portal. You can see all regions here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions;  
> 2.3. **LanguageID**: Language to apply lozalization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
> 
> The **VoiceToTextAsync** function return a delegate to manage task status which allow you to bind a function to it's delegate to handle task's completion call.

> ## Text-to-WAV:
>```
> void AMyExampleActor::Example()
> {
>   FAzSpeechData MyNewData;
>   MyNewData.LanguageID = "LanguageID";
>   MyNewData.RegionID = "RegionID";
>   MyNewData.APIAccessKey = "APIAccessKey";
>
>   UTextToWavAsync* MyTask = UTextToWavAsync::TextToWavAsync(this, "Text to Convert",
>		                                                          "File Path", "File Name",
>		                                                          "Voice Name", MyNewData);
>
>   MyTask->TaskCompleted.AddDynamic(this, &AMyExampleActor::ExampleCallback);
>
>   MyTask->Activate();
> }
>
> void AMyExampleActor::ExampleCallback(const bool TaskResult)
> {
> }
>```
> 
>  1. **Text to WAV Async**: Will convert the specified text to a .wav file.  
> 1.1. **WorldContextObject**: Task Owner.  
> 1.2. **TextToConvert**: The text that will be converted to audio;  
> 1.3. **FilePath**: Output path to save the audio file;  
> 1.4. **FileName**: Output file name;  
> 1.5. **VoiceName**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
> 1.6. **FAzSpeechData**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  
>
> 2. **FAzSpeechData**: Represents Microsoft Azure parameters to connect to the service and perform the tasks;  
> 2.1. **APIAccessKey**: It's your Speech Service API Access Key from your Microsoft Azure Portal - Speech Service Panel;  
> 2.2. **RegionID**: Speech Service Region from your Microsoft Azure Portal. You can see all regions here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions;  
> 2.3. **LanguageID**: Language to apply lozalization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
>
> The **TextToWavAsync** function return a delegate to manage task status which allow you to bind a function to it's delegate to handle task's completion call.

# More informations:
## How to get the Speech Service API Access Key and Region ID:
![image](https://user-images.githubusercontent.com/77353979/157915218-c636d31c-7f7d-4d89-8842-708a6bfbe9c5.png)

See the official Microsoft documentation for Azure Cognitive Services: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/

## Extra: How to get a audio file at runtime (Editor Only)
**Add the module "AudioEditor" to your Build.cs**  
**AudioCompEx is a UAudioComponent**  
**includes: "Sound/SoundWave.h", "Components/AudioComponent.h" and "Factories/SoundFactory.h"**  
```
void Foo()
{
  USoundFactory* NewFactory = NewObject<USoundFactory>(this, TEXT("Sound Factory"));

  bool bOperationCanceled;
  UObject* MyRuntimeObject = NewFactory->FactoryCreateFile(USoundWave::StaticClass(), this, "RuntimeAudio", 
                                                          EObjectFlags::RF_NoFlags, TEXT("FILEPATH"), 
                                                          TEXT(""), nullptr, bOperationCanceled);

  USoundWave* MyRuntimeAudio = Cast<USoundWave>(MyRuntimeObject);
  if (IsValid(MyRuntimeAudio))
  {
    AudioCompEx->SetSound(MyRuntimeAudio);
    AudioCompEx->Play();
  }
}
```
