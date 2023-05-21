// Copyright Epic Games, Inc. All Rights Reserved.

#include "BraneEngineExporterCommands.h"

#define LOCTEXT_NAMESPACE "FBraneEngineExporterModule"

void FBraneEngineExporterCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "BraneEngineExporter", "Bring up BraneEngineExporter window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
