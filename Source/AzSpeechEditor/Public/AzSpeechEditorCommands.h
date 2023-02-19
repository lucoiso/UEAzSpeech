// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "AzSpeechEditorStyle.h"

class FAzSpeechEditorCommands : public TCommands<FAzSpeechEditorCommands>
{
public:

	FAzSpeechEditorCommands()
		: TCommands<FAzSpeechEditorCommands>(TEXT("AzSpeechEditor"), NSLOCTEXT("Contexts", "AzSpeechEditor", "AzSpeechEditor Plugin"), NAME_None, FAzSpeechEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};