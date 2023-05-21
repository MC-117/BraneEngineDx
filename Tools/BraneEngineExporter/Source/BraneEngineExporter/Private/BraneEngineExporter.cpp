// Copyright Epic Games, Inc. All Rights Reserved.

#include "BraneEngineExporter.h"

#include "BraneActor.h"
#include "BraneEngineConverter.h"
#include "BraneEngineExporterStyle.h"
#include "BraneEngineExporterCommands.h"
#include "LevelEditor.h"
#include "ToolMenus.h"

static const FName BraneEngineExporterTabName("BraneEngineExporter");

#define LOCTEXT_NAMESPACE "FBraneEngineExporterModule"

void FBraneEngineExporterModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FBraneEngineExporterStyle::Initialize();
	FBraneEngineExporterStyle::ReloadTextures();

	FBraneEngineExporterCommands::Register();

	Converter = MakeShareable(new Brane::BConverter);
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FBraneEngineExporterCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FBraneEngineExporterModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBraneEngineExporterModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(BraneEngineExporterTabName, FOnSpawnTab::CreateRaw(Converter.Get(), &Brane::BConverter::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FBraneEngineExporterTabTitle", "BraneEngineExporter"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FBraneEngineExporterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FBraneEngineExporterStyle::Shutdown();

	FBraneEngineExporterCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BraneEngineExporterTabName);
}

void FBraneEngineExporterModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(BraneEngineExporterTabName);
}

void FBraneEngineExporterModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FBraneEngineExporterCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FBraneEngineExporterCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBraneEngineExporterModule, BraneEngineExporter)