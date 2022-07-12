# Unreal Engine Plugin: AzSpeech

## Table of Contents
> 1. [About](#about)  
> 2. [Links](#links)  
> 3. [Documentation](#documentation)  
> 3.1. [Installation](#installation)  
> 3.2. [Blueprint: Usage](#blueprint-usage)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.1. [Blueprint: Voice to Text](#voice-to-text)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.2. [Blueprint: Text to Voice](#text-to-voice)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.3. [Blueprint: Text to WAV](#text-to-wav)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.4. [Blueprint: WAV to Text](#wav-to-text)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.5. [Blueprint: Text to Stream](#text-to-stream)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.6. [Blueprint: SSML to Voice](#ssml-to-voice)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.7. [Blueprint: SSML to WAV](#ssml-to-wav)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.8. [Blueprint: SSML to Stream](#ssml-to-stream)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.9. [Blueprint: Make AzSpeechData](#make-azspeechdata)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.10. [Blueprint: Convert File to Sound Wave](#convert-file-to-sound-wave)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.11. [Blueprint: Convert Stream to Sound Wave](#convert-stream-to-sound-wav)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.12. [Blueprint: Is AzSpeechData Empty](#is-azspeechdata-empty)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.13. [Blueprint: Qualify Path](#qualify-path)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.14. [Blueprint: Qualify WAV File Path](#qualify-wav-file-path)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.15. [Blueprint: Qualify XML File Path](#qualify-xml-file-path)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.16. [Blueprint: Load XML to String](#load-xml-to-string)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.2.16. [Blueprint: Load XML to String](#load-xml-to-string)  
> 3.3 [C++: Usage](#c-usage)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.3.1 [C++: Setup](#setup)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.3.2. [C++: Voice to Text](#voice-to-text-1)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.3.3. [C++: Text to Voice](#text-to-voice-1)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.3.4. [C++: Text to WAV](#text-to-wav-1)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.3.5. [C++: WAV to Text](#wav-to-text-1)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.3.6. [C++: Text to Stream](#text-to-stream-1)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.3.7. [C++: SSML to Voice](#ssml-to-voice-1)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.3.8. [C++: SSML to WAV](#ssml-to-wav-1)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.3.9. [C++: SSML to Stream](#ssml-to-stream-1)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.3.10. [C++: Helper Functions](#helper-functions)  
> 4. [More Information](#more-information)  
> 4.1. [How to get the Speech Service API Access Key and Region ID](#how-to-get-the-speech-service-api-access-key-and-region-id)  

## About
![image](https://user-images.githubusercontent.com/77353979/178588725-fc2b951d-64a1-4ec0-8f29-11fe50e26773.png)

A plugin integrating Azure Speech Cognitive Services to Unreal Engine with simple functions which can do these asynchronous tasks: 
* Voice-To-Text (Convert a speech to a string)
* Text-To-Voice (Convert a string to a speech)
* Text-To-Wav (Convert a string to a .wav audio file)
* Text-To-Stream (Convert a string to a audio data stream)
* Wav-To-Text (Convert a .wav audio file to a string)
* SSML-To-Voice (Convert a SSML file to speech)
* SSML-To-Wav (Convert a SSML file to a .wav audio file)
* SSML-To-Stream (Convert a SSML file to a audio data stream)

And helper functions:
* Convert File to Sound Wave: Runtime USoundWave importer via Audio File
* Convert Stream to Sound Wave: Runtime USoundWave importer via Audio Data Stream
* Is AzSpeechData Empty: Check if the passed data has empty values
* Load XML to String: Load a XML file and pass its content to a string
* Qualify Path: Check if the path ends with '/'
* Qualify XML File Path: Check if the filepath ends with the correct extension ".xml"
* Qualify WAV File Path: Check if the filepath ends with the correct extension ".wav"

## Links

* [UE Marketplace](https://www.unrealengine.com/marketplace/en-US/product/azspeech-async-text-to-voice-and-voice-to-text)  
* [Unreal Engine Forum](https://forums.unrealengine.com/t/free-azspeech-plugin-async-text-to-voice-and-voice-to-text-with-microsoft-azure/495394)  
* [Microsoft Documentation](https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/)  

# Documentation
## Installation
Just clone this repository or download the .zip file in the latest version and drop the files inside the 'Plugins/' folder of your project folder.
Note that if your project doesn't have a 'Plugins' folder, you can create one.

## Blueprint Usage
> ## Voice to Text
> ![image](https://user-images.githubusercontent.com/77353979/157915810-15434664-5681-4538-89c2-801df6934749.png)  
> **Voice to Text Async**: Will convert your speech to string. Not that this function will get the speech by your default sound input device.  
> **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  

> ## Text to Voice
> ![image](https://user-images.githubusercontent.com/77353979/157915746-b6681afa-6bce-447e-acc9-1bcb6824db51.png)  
> **Text to Voice Async**: Will convert the specified text to a sound that will be played by the default sound output device.  
> **Text to Convert**: The text that will be converted to audio;  
> **Voice Name**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
> **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  

> ## Text to WAV
> ![image](https://user-images.githubusercontent.com/77353979/157916050-6a691103-d143-4296-a561-21471dbf9e57.png)  
> **Text to WAV Async**: Will convert the specified text to a .wav file.  
> **Text to Convert**: The text that will be converted to audio;  
> **File Path**: Output path to save the audio file;  
> **File Name**: Output file name;  
> **Voice Name**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
> **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  

> ## WAV to Text
> ![image](https://user-images.githubusercontent.com/77353979/164303053-4a08911c-0e67-4e74-b825-d8e6eabccc8d.png)  
> **WAV to Text Async**: Will convert the specified .wav file to a string.  
> **File Path**: Input path of the audio file;  
> **File Name**: Input file name;  
> **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  

> ## Text to Stream
> ![image](https://user-images.githubusercontent.com/77353979/164303195-186a9b3c-9d10-45a6-bc13-86b6bb7fe0fd.png)  
> **Text to Stream Async**: Will convert the specified text to a data stream.  
> **Text to Convert**: The text that will be converted to stream;  
> **Voice Name**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
> **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  

> ## SSML to Voice
> ![image](https://user-images.githubusercontent.com/77353979/178589873-79a0ef15-ce19-4b95-85a5-ec26ae842834.png)  
> **SSML to Voice Async**: Will convert the specified SSML String to a sound that will be played by the default sound output device.  
> **SSMLString**: The content of the XML (SSML) file.
> **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  

> ## SSML to Wav
> ![image](https://user-images.githubusercontent.com/77353979/178589891-ba1c2b50-69e6-4be5-b95c-ddcf0f13d9a3.png)  
> **SSML to Wav Async**: Will convert the specified SSML String to a .wav file.  
> **SSMLString**: The content of the XML (SSML) file.
> **File Path**: Output path to save the audio file;  
> **File Name**: Output file name;  
> **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  

> ## SSML to Stream
> ![image](https://user-images.githubusercontent.com/77353979/178589907-ccdd06db-7f3b-415c-a521-610864e8ffd4.png)  
> **SSML to Stream Async**: Will convert the specified SSML String to a data stream.
> **SSMLString**: The content of the XML (SSML) file.
> **Parameters**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  

> ## Make AzSpeechData
>![image](https://user-images.githubusercontent.com/77353979/157916110-9e6a89f6-da94-46ef-bc78-287830bc3e7f.png)  
> **AzSpeechData**: Represents Microsoft Azure parameters to connect to the service and perform the tasks;  
> **API Access Key**: It's your Speech Service API Access Key from your Microsoft Azure Portal - Speech Service Panel;  
> **Region ID**: Speech Service Region from your Microsoft Azure Portal. You can see all regions here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions;  
> **Language ID**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 

> ## Convert File To Sound Wave
> ![image](https://user-images.githubusercontent.com/77353979/166697585-4ec0e9b2-447d-42b5-a745-812b3fbcd086.png)  
> **Convert File To Sound Wave**: Will load a specified audio file and transform into a transient USoundWave.  
> **File Path**: Input path of the audio file;  
> **File Name**: Input file name;  
 
> ## Convert Stream To Sound Wav
> ![image](https://user-images.githubusercontent.com/77353979/166697554-55ba6118-8a20-4a7b-9dd9-0f85e0961ee1.png)  
> **Convert Stream To Sound Wave**: Will convert the specified data stream into a transient USoundWave.  
> **Raw Data**: Data stream of a specified audio.

> ## Is AzSpeechData Empty
> ![image](https://user-images.githubusercontent.com/77353979/178590246-b01433e0-b31b-4434-adf8-5e94be28f2ee.png)  
> **Is AzSpeechData Empty**: Check if the passed data has empty values.  
> **Data**: The FAzSpeechData to check  

> ## Qualify Path
> ![image](https://user-images.githubusercontent.com/77353979/178590380-f59860f8-b2ca-4563-b693-78ca425c73bc.png)  
> **Qualify Path**: A helper function to check if the given path ends with '/' and adjust it if not  
> **Path**: The path to check  

> ## Qualify WAV File Path
> ![image](https://user-images.githubusercontent.com/77353979/178590819-b2ec56c1-699b-40ea-b124-69fd3e8e42f1.png)  
> **Qualify WAV File Path**: A helper function to check if the given file path ends with ".wav" and adjust it if not  
> **File Path**: The file path to check  
> **File Name**: The file name to check  

> ## Qualify XML File Path
> ![image](https://user-images.githubusercontent.com/77353979/178590833-94e7e979-518e-403e-8dc6-044ecd3281ff.png)  
> **Qualify XML File Path**: A helper function to check if the given path ends with ".xml" and adjust it if not  
> **File Path**: The file path to check  
> **File Name**: The file name to check  

> ## Load XML to String
> ![image](https://user-images.githubusercontent.com/77353979/178590845-c24c43c7-760e-40f3-8450-32a7b0daf1b6.png)  
> **Load XML to String**: Load a .xml file and get its content as string  
> **File Path**: The file path to load  
> **File Name**: The file name to load  

## C++ Usage
## Setup
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
> 2.3. **LanguageID**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
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
> 2.3. **LanguageID**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
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
>  1. **UTextToWavAsync::TextToWavAsync(WorldContextObject, TextToConvert, FilePath, FileName, FAzSpeechData)**: Will convert the specified text to a .wav file.  
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
> 2.3. **LanguageID**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
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
> 2.3. **LanguageID**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
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
> 2.3. **LanguageID**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
> 
> The **TextToStreamAsync** function return a delegate to manage task status which allow you to bind a function to it's delegate to handle task's completion call.

> ## SSML-to-Voice
>```
> #include "AzSpeech/SSMLToVoiceAsync.h"
> 
> void AMyExampleActor::Example()
> {
>   FAzSpeechData MyNewData;
>   MyNewData.LanguageID = "LanguageID";
>   MyNewData.RegionID = "RegionID";
>   MyNewData.APIAccessKey = "APIAccessKey";
>
>   USSMLToVoiceAsync* MyTask = USSMLToVoiceAsync::SSMLToVoiceAsync(this, "SSMLString", MyNewData);
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
> 1. **UTextToStreamAsync::TextToStreamAsync(WorldContextObject, TextToConvert, VoiceName, FAzSpeechData)**: Will convert the specified text file into a audio data stream.  
> 1.1. **WorldContextObject**: Task Owner.  
> 1.2. **SSMLString**: The SSML content that will be converted to speech;  
> 1.3. **FAzSpeechData**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  
> 
> 2. **FAzSpeechData**: Represents Microsoft Azure parameters to connect to the service and perform the tasks;  
> 2.1. **APIAccessKey**: It's your Speech Service API Access Key from your Microsoft Azure Portal - Speech Service Panel;  
> 2.2. **RegionID**: Speech Service Region from your Microsoft Azure Portal. You can see all regions here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions;  
> 2.3. **LanguageID**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
> 
> The **SSMLToVoiceAsync** function return a delegate to manage task status which allow you to bind a function to it's delegate to handle task's completion call.
 
> ## SSML-to-WAV
>```
> #include "AzSpeech/SSMLToWavAsync.h"
>
> void AMyExampleActor::Example()
> {
>   FAzSpeechData MyNewData;
>   MyNewData.LanguageID = "LanguageID";
>   MyNewData.RegionID = "RegionID";
>   MyNewData.APIAccessKey = "APIAccessKey";
>
>   USSMLToWavAsync* MyTask = USSMLToWavAsync::SSMLToWavAsync(this, "SSMLString",
>		                                                          "File Path", "File Name",
>		                                                          MyNewData);
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
>  1. **USSMLToWavAsync::SSMLToWavAsync(WorldContextObject, SSMLString, FilePath, FileName, FAzSpeechData)**: Will convert the specified SSML content to a .wav file.  
> 1.1. **WorldContextObject**: Task Owner.  
> 1.2. **SSMLString**: The SSML content that will be converted to audio;  
> 1.3. **FilePath**: Output path to save the audio file;  
> 1.4. **FileName**: Output file name;  
> 1.5. **FAzSpeechData**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  
>
> 2. **FAzSpeechData**: Represents Microsoft Azure parameters to connect to the service and perform the tasks;  
> 2.1. **APIAccessKey**: It's your Speech Service API Access Key from your Microsoft Azure Portal - Speech Service Panel;  
> 2.2. **RegionID**: Speech Service Region from your Microsoft Azure Portal. You can see all regions here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions;  
> 2.3. **LanguageID**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
>
> The **SSMLToWavAsync** function return a delegate to manage task status which allow you to bind a function to it's delegate to handle task's completion call.
 
> ## SSML-to-Stream
>```
> #include "AzSpeech/SSMLToStreamAsync.h"
> 
> void AMyExampleActor::Example()
> {
>   FAzSpeechData MyNewData;
>   MyNewData.LanguageID = "LanguageID";
>   MyNewData.RegionID = "RegionID";
>   MyNewData.APIAccessKey = "APIAccessKey";
>
>   USSMLToStreamAsync* MyTask = USSMLToStreamAsync::TextToStreamAsync(this, "SSMLString", MyNewData);
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
> 1. **USSMLToStreamAsync::SSMLToStreamAsync(WorldContextObject, SSMLString, FAzSpeechData)**: Will convert the specified SSML content into a audio data stream.  
> 1.1. **WorldContextObject**: Task Owner.  
> 1.2. **SSMLString**: The SSML content that will be converted to data stream;  
> 1.3. **FAzSpeechData**: Microsoft Azure parameters to connect to the service and perform the tasks. The structure **AzSpeechData** represents this input;  
> 
> 2. **FAzSpeechData**: Represents Microsoft Azure parameters to connect to the service and perform the tasks;  
> 2.1. **APIAccessKey**: It's your Speech Service API Access Key from your Microsoft Azure Portal - Speech Service Panel;  
> 2.2. **RegionID**: Speech Service Region from your Microsoft Azure Portal. You can see all regions here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/regions;  
> 2.3. **LanguageID**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text; 
> 
> The **SSMLToStreamAsync** function return a delegate to manage task status which allow you to bind a function to it's delegate to handle task's completion call.

> ## Helper Functions
> All helper functions are commented and declared inside the *"AzSpeech/AzSpeechHelper.h"* file.
 
# More information
## How to get the Speech Service API Access Key and Region ID
![image](https://user-images.githubusercontent.com/77353979/157915218-c636d31c-7f7d-4d89-8842-708a6bfbe9c5.png)

See the official Microsoft documentation for Azure Cognitive Services: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/
