# Plugin: AzSpeech
## About the project

A plugin integrating Azure Speech Cognitive Services to Unreal Engine with simple functions which can do these asynchronous tasks: 
  1. Voice-To-Text (Convert a speech into a string)
  2. Text-To-Voice (Convert a string into a speech)
  3. Text-To-Wav (Convert a string into a .wav audio file)

## Links

**Marketplace:** [AzSpeech - Text and Voice
](https://www.unrealengine.com/marketplace/en-US/product/azspeech-async-text-to-voice-and-voice-to-text)  
**Forum:** [[FREE] AzSpeech plugin: Async Text-to-Voice and Voice-to-Text with Microsoft Azure](https://forums.unrealengine.com/t/free-azspeech-plugin-async-text-to-voice-and-voice-to-text-with-microsoft-azure/495394)  
**Microsoft Documentation:** [Speech Service Documentation](https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/)  

# Documentation
## Installation
Just clone this repository or download the .zip file in the latest version and drop the files inside the 'Plugins/' folder of your project folder.
Note that if your project doesn't have a 'Plugins' folder, you can create one.

## Blueprint Usage

![image](https://user-images.githubusercontent.com/77353979/164302957-90595cbb-2548-40b2-89da-7db118614dde.png)  
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
>  3.1. **Text to Convert**: The text that will be converted to audio;  
>  3.2. **File Path**: Output path to save the audio file;  
>  3.3. **File Name**: Output file name;  
>  3.4. **Voice Name**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
>  3.5. **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  

> ![image](https://user-images.githubusercontent.com/77353979/164303053-4a08911c-0e67-4e74-b825-d8e6eabccc8d.png)  
>  4. **WAV to Text Async**: Will convert the specified .wav file into a string.  
>  4.1. **File Path**: Input path of the audio file;  
>  4.2. **File Name**: Input file name;  
>  4.3. **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  

> ![image](https://user-images.githubusercontent.com/77353979/164303195-186a9b3c-9d10-45a6-bc13-86b6bb7fe0fd.png)  
>  5. **Text to Stream Async**: Will convert the specified text to a data stream.  
>  5.1. **Text to Convert**: The text that will be converted to stream;  
>  5.2. **Voice Name**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
>  5.3. **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  

>![image](https://user-images.githubusercontent.com/77353979/157916110-9e6a89f6-da94-46ef-bc78-287830bc3e7f.png)  
> 6. **AzSpeechData**: Represents Microsoft Azure parameters to connect to the service and perform the tasks;  
> 6.1. **API Access Key**: It's your Speech Service API Access Key from your Microsoft Azure Portal - Speech Service Panel;  
> 6.2. **Region ID**: Speech Service Region from your Microsoft Azure Portal. You can see all regions here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions;  
> 6.3. **Language ID**: Language to apply lozalization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 

> ![image](https://user-images.githubusercontent.com/77353979/164303797-0787565a-4252-4528-9709-e6458e519702.png)  
> 7. **Convert File Into Sound Wave**: Will load a specified audio file and transform into a transient USoundWave.  
>  7.1. **File Path**: Input path of the audio file;  
>  7.2. **File Name**: Input file name;  
 
> ![image](https://user-images.githubusercontent.com/77353979/164303943-c5c0456d-70ab-4aed-8ffa-e0daa289c461.png)  
> 8. **Convert Stream into a Sound Wave**: Will convert the specified data stream into a transient USoundWave.
> 8.1 **Raw Data**: Data stream of a specified audio.

## C++ Usage

**You need to include the module "AzSpeech" inside your .Build.cs class to be allowed to call AzSpeech functions from C++.**
![image](https://user-images.githubusercontent.com/77353979/157924008-6ce0c137-3c21-4c82-b0b9-de1a0ab3ac9f.png)

> ## Text-to-Voice
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

> ## Voice-to-Text
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

> ## Text-to-WAV
>```
> #include "AzSpeech/TextToWavAsync.h"
>
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

> ## WAV-to-Text
>```
> #include "AzSpeech/WavToTextAsync.h"
> 
> void AMyExampleActor::Example()
> {
>   FAzSpeechData MyNewData;
>   MyNewData.LanguageID = "LanguageID";
>   MyNewData.RegionID = "RegionID";
>   MyNewData.APIAccessKey = "APIAccessKey";
>
>   UWavToTextAsync* MyTask = UWavToTextAsync::WavToTextAsync(this, "FilePath", "FileName", MyNewData);
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
> 1. **UWavToTextAsync::WavToTextAsync(WorldContextObject, FilePath, FileName, FAzSpeechData)**: Will convert the specified .wav file into a string.  
> 1.1. **WorldContextObject**: Task Owner.  
> 1.2. **FilePath**: Input path of the audio file.  
> 1.3. **FileName**: Input file name.    
> 1.4. **FAzSpeechData**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  
> 
> 2. **FAzSpeechData**: Represents Microsoft Azure parameters to connect to the service and perform the tasks;  
> 2.1. **APIAccessKey**: It's your Speech Service API Access Key from your Microsoft Azure Portal - Speech Service Panel;  
> 2.2. **RegionID**: Speech Service Region from your Microsoft Azure Portal. You can see all regions here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions;  
> 2.3. **LanguageID**: Language to apply lozalization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
> 
> The **WavToTextAsync** function return a delegate to manage task status which allow you to bind a function to it's delegate to handle task's completion call.

> ## Text-to-Stream
>```
> #include "AzSpeech/TextToStreamAsync.h"
> 
> void AMyExampleActor::Example()
> {
>   FAzSpeechData MyNewData;
>   MyNewData.LanguageID = "LanguageID";
>   MyNewData.RegionID = "RegionID";
>   MyNewData.APIAccessKey = "APIAccessKey";
>
>   UTextToStreamAsync* MyTask = UTextToStreamAsync::TextToStreamAsync(this, "TextToConvert", "VoiceName", MyNewData);
>
>   MyTask->TaskCompleted.AddDynamic(this, &AMyExampleActor::ExampleCallback);
>
>   MyTask->Activate();
> }
>
> void AMyExampleActor::ExampleCallback(const TArray<uint8>& TaskResult)
> {
> }
>```
>
> 1. **UTextToStreamAsync::TextToStreamAsync(WorldContextObject, TextToConvert, VoiceName, FAzSpeechData)**: Will convert the specified text file into a audio data stream.  
> 1.1. **WorldContextObject**: Task Owner.  
> 1.2. **TextToConvert**: The text that will be converted to data stream;  
> 1.3. **VoiceName**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
> 1.4. **FAzSpeechData**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  
> 
> 2. **FAzSpeechData**: Represents Microsoft Azure parameters to connect to the service and perform the tasks;  
> 2.1. **APIAccessKey**: It's your Speech Service API Access Key from your Microsoft Azure Portal - Speech Service Panel;  
> 2.2. **RegionID**: Speech Service Region from your Microsoft Azure Portal. You can see all regions here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions;  
> 2.3. **LanguageID**: Language to apply lozalization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
> 
> The **TextToStreamAsync** function return a delegate to manage task status which allow you to bind a function to it's delegate to handle task's completion call.


# More informations:
## How to get the Speech Service API Access Key and Region ID:
![image](https://user-images.githubusercontent.com/77353979/157915218-c636d31c-7f7d-4d89-8842-708a6bfbe9c5.png)

See the official Microsoft documentation for Azure Cognitive Services: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/
