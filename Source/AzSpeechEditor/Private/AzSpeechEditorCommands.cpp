// Copyright Epic Games, Inc. All Rights Reserved.

#include "AzSpeechEditorCommands.h"

#define LOCTEXT_NAMESPACE "FAzSpeechEditorModule"

void FAzSpeechEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "AzSpeechEditor", "Bring up AzSpeechEditor window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
