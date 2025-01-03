#include "resource.h"
#include "Core/Engine.h"
#include "Tool/ToolShelf.h"
#include "Tool/AnimationConverter/AnimationConverter.h"
#include "Core/GUI/ConsoleWindow.h"
#include "Core/Timeline/TimelineWindow.h"
#include "Core/Application.h"
#include "Core/EngineLoop/GUIEngineLoop.h"
#include "Core/Graph/GraphWindow.h"
#include "Core/Importer/Importer.h"
#include "Core/GUI/ShaderManagerWindow.h"
#include "ImPlot/implot.h"
#include <mimalloc-new-delete.h>

class ImPlotWindow : public UIWindow
{
public:
	ImPlotWindow(string name = "ImPlot", bool defaultShow = false)
		: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
	{
		
	}

	virtual void onGUI(GUIRenderInfo& info)
	{
		ImPlot::ShowDemoWindow(&show);
	}
};

class ToolApplication : public EngineWindowApplication
{
public:
	ToolApplication() : EngineWindowApplication(MAKEINTRESOURCE(IDI_ICON1)) {}
	
	virtual void loadAssets()
	{
		ShaderManager::loadDefaultAdapter("Engine/Shaders/Default");

		if (!Material::loadDefaultMaterial()) {
			MessageBox(NULL, "Default material load failed", "Error", MB_OK);
		}

		IImporter::loadFolder("Engine/Shaders");
	}
	
	virtual void initializeEngineLoop()
	{
		toolShelf.registTool(*new ConsoleWindow(*Engine::getCurrentWorld()));
		toolShelf.registTool(*new AnimationConverter());
		toolShelf.registTool(*new TimelineWindow());
		toolShelf.registTool(*new ImPlotWindow());
		toolShelf.registTool(*new GraphWindow());

		GUI::get().setMainControl(&toolShelf);
		engine.setEngineLoop(*new GUIOnlyEngineLoop(GUI::get()));
		Engine::get().getEngineLoop()->init();
	}
protected:
	ToolShelf toolShelf;
};

RegisterApplication(ToolApplication);

// void InitialWorld() {
// 	World& world = *Engine::getCurrentWorld();
// 	world.setGUIOnly(true);
// 	Material& pbr_mat = *getAssetByPath<Material>("Engine/Shaders/PBR.mat");
// 	ToolShelf& toolShelf = *new ToolShelf("ToolShelf", true);
// 	toolShelf.showCloseButton = false;
// 	world += toolShelf;
//
// 	toolShelf.registTool(*new ConsoleWindow(*Engine::getCurrentWorld()));
// 	toolShelf.registTool(*new AnimationConverter());
// 	toolShelf.registTool(*new TimelineWindow());
// }