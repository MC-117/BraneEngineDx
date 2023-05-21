// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "BraneEngineExporterStyle.h"

class FBraneEngineExporterCommands : public TCommands<FBraneEngineExporterCommands>
{
public:

	FBraneEngineExporterCommands()
		: TCommands<FBraneEngineExporterCommands>(TEXT("BraneEngineExporter"), NSLOCTEXT("Contexts", "BraneEngineExporter", "BraneEngineExporter Plugin"), NAME_None, FBraneEngineExporterStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};