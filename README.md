# Plugin: AzSpeech
## The project:

A plugin integrating Azure Speech Cognitive Services to Unreal Engine with only 2 simple functions which can do these asynchronous tasks: 
1. TextToVoiceAsync
2. VoiceToTextAsync

Post on Unreal Engine Forum: https://forums.unrealengine.com/t/free-azspeech-plugin-async-text-to-voice-and-voice-to-text-with-microsoft-azure/495394

# Installation
Just clone this repository or download the .zip file in the latest version and drop the files inside the 'Plugins/' folder of your project folder.
Note that if your project doesn't have a 'Plugins' folder, you can create one.

# Examples
## Blueprint Nodes:

![AzSpeechBp1](https://user-images.githubusercontent.com/77353979/154168457-909aa90a-c816-40a7-b627-0404044b7f34.png)


## C++ Implementation (Text-to-Voice):

![image](https://user-images.githubusercontent.com/77353979/154168468-7ef7fda0-f47c-4a8e-b9a2-fef94a59073b.png)

_(Using GameMode class only for demonstration)_

## C++ Implementation (Voice-to-Text):

![image](https://user-images.githubusercontent.com/77353979/154168475-f277e85d-7d1a-482a-9d09-97f0947bad1d.png)

_(Using GameMode class only for demonstration)_

# More infos on the way...

See the official Microsoft documentation for Azure Cognitive Services: https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/
