# Unreal Engine Plugin: AzSpeech

## Table of Contents
> 1. [About](#about)  
> 2. [Links](#links)  
> 3. [Documentation](#documentation)  
> 3.1. [Installation](#installation)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.1. [Voice to Text](#voice-to-text)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.2. [Text to Voice](#text-to-voice)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.3. [Text to WAV](#text-to-wav)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.4. [WAV to Text](#wav-to-text)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.5. [Text to Stream](#text-to-stream)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.6. [SSML to Voice](#ssml-to-voice)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.7. [SSML to WAV](#ssml-to-wav)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.8. [SSML to Stream](#ssml-to-stream)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.9. [Convert File to Sound Wave](#convert-file-to-sound-wave)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.10. [Convert Stream to Sound Wave](#convert-stream-to-sound-wave)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.11. [Qualify Path](#qualify-path)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.12. [Qualify WAV File Path](#qualify-wav-file-path)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.13. [Qualify XML File Path](#qualify-xml-file-path)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.14. [Qualify File Extension](#qualify-file-extension)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.15. [Create New Directory](#create-new-directory)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.16. [Load XML to String](#load-xml-to-string)  
> 4. [Setup](#setup)  
> 4.1. [Setup: C++ only](#setup-c-only)  
> 5. [More Information](#more-information)  
> 5.1. [How to get the Speech Service API Access Key and Region ID](#how-to-get-the-speech-service-api-access-key-and-region-id)  

## About
![image](https://user-images.githubusercontent.com/77353979/182722300-9a85d697-53d0-46d3-a511-1503bd3a0511.png)

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
* Load XML to String: Load a XML file and pass its content to a string
* Qualify Path: Check if the path ends with '/'
* Qualify XML File Path: Check if the filepath ends with the correct extension ".xml"
* Qualify WAV File Path: Check if the filepath ends with the correct extension ".wav"
* Qualify File Extension: Check if the file ends with the correct extension
* Create New Directory: A helper function to create a new folder

## Links

* [UE Marketplace](https://www.unrealengine.com/marketplace/en-US/product/azspeech-async-text-to-voice-and-voice-to-text)  
* [Unreal Engine Forum](https://forums.unrealengine.com/t/free-azspeech-plugin-async-text-to-voice-and-voice-to-text-with-microsoft-azure/495394)  
* [Microsoft Documentation](https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/)  

# Documentation
## Installation
Just clone this repository or download the .zip file in the latest version and drop the files inside the 'Plugins/' folder of your project folder.
Note that if your project doesn't have a 'Plugins' folder, you can create one.

> ## Voice to Text
> ![image](https://user-images.githubusercontent.com/77353979/182720080-fecf20bf-d0f9-443d-8a5f-44e24c8b48f3.png)  
> **Voice to Text**: Will convert your speech to string. Not that this function will get the speech by your default sound input device;  
> **Language Id**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text;  
> **C++ Header**: #include "AzSpeech/VoiceToTextAsync.h"  

> ## Text to Voice
> ![image](https://user-images.githubusercontent.com/77353979/182720104-f1cff546-2f05-4993-8bef-33bdb2474f1b.png)  
> **Text to Voice**: Will convert the specified text to a sound that will be played by the default sound output device;  
> **Text to Convert**: The text that will be converted to audio;  
> **Voice Name**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
> **Language Id**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text;  
> **C++ Header**: #include "AzSpeech/TextToVoiceAsync.h"  

> ## Text to WAV
> ![image](https://user-images.githubusercontent.com/77353979/182720125-43b4b6ab-b5c2-47ff-9044-1a5b67dc05d3.png)  
> **Text to WAV**: Will convert the specified text to a .wav file;  
> **Text to Convert**: The text that will be converted to audio;  
> **File Path**: Output path to save the audio file;  
> **File Name**: Output file name;  
> **Voice Name**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
> **Language Id**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text;  
> **C++ Header**: #include "AzSpeech/TextToWavAsync.h";  

> ## WAV to Text
> ![image](https://user-images.githubusercontent.com/77353979/182720150-dbf4d23e-58b6-4cd2-a617-82321b65af45.png)  
> **WAV to Text**: Will convert the specified .wav file to a string;  
> **File Path**: Input path of the audio file;  
> **File Name**: Input file name;  
> **Language Id**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text;  
> **C++ Header**: #include "AzSpeech/WavToTextAsync.h";  

> ## Text to Stream
> ![image](https://user-images.githubusercontent.com/77353979/182720176-b5ba0b29-3972-4c1c-a118-bc3530568144.png)  
> **Text to Stream**: Will convert the specified text to a data stream;  
> **Text to Convert**: The text that will be converted to stream;  
> **Voice Name**: Voice code that will represent the type of voice that will "read" the converted text. You can see all names here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#text-to-speech;  
> **Language Id**: Language to apply localization settings. You can see all IDs here: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/language-support#speech-to-text;  
> **C++ Header**: #include "AzSpeech/TextToStreamAsync.h";  

> ## SSML to Voice
> ![image](https://user-images.githubusercontent.com/77353979/182720194-b0f7ca52-0cc1-4a6b-9721-19902c196fcc.png)  
> **SSML to Voice**: Will convert the specified SSML String to a sound that will be played by the default sound output device;  
> **SSMLString**: The content of the XML (SSML) file;
> **C++ Header**: #include "AzSpeech/SSMLToVoiceAsync.h";  

> ## SSML to Wav
> ![image](https://user-images.githubusercontent.com/77353979/182720214-a67728f4-6017-4c66-9d76-489a077cf7e8.png)  
> **SSML to Wav**: Will convert the specified SSML String to a .wav file;  
> **SSMLString**: The content of the XML (SSML) file;
> **File Path**: Output path to save the audio file;  
> **File Name**: Output file name;  
> **C++ Header**: #include "AzSpeech/SSMLToWavAsync.h"; 

> ## SSML to Stream
> ![image](https://user-images.githubusercontent.com/77353979/182720243-75de078e-8c21-404a-a322-140410dac917.png)  
> **SSML to Stream**: Will convert the specified SSML String to a data stream;
> **SSMLString**: The content of the XML (SSML) file;  
> **C++ Header**: #include "AzSpeech/SSMLToStreamAsync.h"; 

> ## Convert File To Sound Wave
> ![image](https://user-images.githubusercontent.com/77353979/182723072-493f1d65-b5ca-46fd-a05c-5796573350ca.png)  
> **Convert File To Sound Wave**: Will load a specified audio file and transform into a transient USoundWave;  
> **File Path**: Input path of the audio file;  
> **File Name**: Input file name;  
> **C++ Header**: #include "AzSpeech/AzSpeechHelper.h";  
 
> ## Convert Stream To Sound Wave
> ![image](https://user-images.githubusercontent.com/77353979/182723096-ba780d62-1cd2-48d0-8501-91ab8ffdb538.png)  
> **Convert Stream To Sound Wave**: Will convert the specified data stream into a transient USoundWave;  
> **Raw Data**: Data stream of a specified audio;  
> **C++ Header**: #include "AzSpeech/AzSpeechHelper.h";  

> ## Qualify Path
> ![image](https://user-images.githubusercontent.com/77353979/182723106-ec5c93c3-146a-4fba-a9ef-2b7f3a282751.png)  
> **Qualify Path**: A helper function to check if the given path ends with '/' and adjust it if not;  
> **Path**: The path to check;  
> **C++ Header**: #include "AzSpeech/AzSpeechHelper.h";  

> ## Qualify WAV File Path
> ![image](https://user-images.githubusercontent.com/77353979/182723117-74ebd99e-23e6-456f-8265-f87952a7277c.png)  
> **Qualify WAV File Path**: A helper function to check if the given file path ends with ".wav" and adjust it if not;  
> **File Path**: The file path to check;  
> **File Name**: The file name to check;  
> **C++ Header**: #include "AzSpeech/AzSpeechHelper.h";  

> ## Qualify XML File Path
> ![image](https://user-images.githubusercontent.com/77353979/182723136-bf6919fb-62a0-4c97-8039-a6510e135493.png)  
> **Qualify XML File Path**: A helper function to check if the given path ends with ".xml" and adjust it if not;  
> **File Path**: The file path to check;  
> **File Name**: The file name to check;  
> **C++ Header**: #include "AzSpeech/AzSpeechHelper.h";  

> ## Qualify File Extension
> ![image](https://user-images.githubusercontent.com/77353979/182723150-56593da1-fd24-44b1-8de0-36eefefbb46c.png)  
> **Qualify XML File Path**: A helper function to check if the given filepath ends with the given extension and adjust it if not;  
> **File Path**: The file path to check;  
> **File Name**: The file name to check;  
> **C++ Header**: #include "AzSpeech/AzSpeechHelper.h";  

> ## Create New Directory
> ![image](https://user-images.githubusercontent.com/77353979/182722163-97ca2b0b-295a-4ecb-9e5e-5cc410b73912.png)  
> **Create New Directory**: A helper function to create a new folder;  
> **File Path**: The file path to create;  
> **Create Parents**: Boolean to determine if the function will create the parents if doesn't exists;  
> **C++ Header**: #include "AzSpeech/AzSpeechHelper.h";  

> ## Load XML to String
> ![image](https://user-images.githubusercontent.com/77353979/182723171-580f7a61-bcd0-4a15-9340-a1e69268f7a5.png)  
> **Load XML to String**: Load a .xml file and get its content as string;  
> **File Path**: The file path to load;  
> **File Name**: The file name to load;  
> **C++ Header**: #include "AzSpeech/AzSpeechHelper.h";  

## Setup
![image](https://user-images.githubusercontent.com/77353979/182721796-0012ac68-bfba-48f7-aa25-8c571d0c3772.png)

Fill the settings with your Microsoft Azure credentials and default parameters.

> ## Setup: C++ only:
> **You need to include the module "AzSpeech" as dependency inside your .Build.cs class to be allowed to call AzSpeech functions from C++.**
 
# More information
## How to get the Speech Service API Access Key and Region ID
![image](https://user-images.githubusercontent.com/77353979/157915218-c636d31c-7f7d-4d89-8842-708a6bfbe9c5.png)

See the official Microsoft documentation for Azure Cognitive Services: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/
