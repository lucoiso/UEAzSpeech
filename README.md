# Unreal Engine Plugin: AzSpeech

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

* [Documentation](https://github.com/lucoiso/UEAzSpeech/wiki)
* [UE Marketplace](https://www.unrealengine.com/marketplace/en-US/product/azspeech-async-text-to-voice-and-voice-to-text)
* [Unreal Engine Forum](https://forums.unrealengine.com/t/free-azspeech-plugin-async-text-to-voice-and-voice-to-text-with-microsoft-azure/495394)
* [Microsoft Documentation](https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/)
