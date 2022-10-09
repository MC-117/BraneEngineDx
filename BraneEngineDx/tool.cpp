#include "Core/Engine.h"
#include "Tool/ToolShelf.h"
#include "Tool/AnimationConverter/AnimationConverter.h"
#include "Core/GUI/ConsoleWindow.h"
#include "Core/GUI/TextureViewer.h"
#include "Core/Timeline/TimelineWindow.h"

void InitialTool() {
	Material& pbr_mat = *getAssetByPath<Material>("Engine/Shaders/PBR.mat");
	ToolShelf& toolShelf = *new ToolShelf("ToolShelf", true);
	toolShelf.showCloseButton = false;
	world += toolShelf;

	toolShelf.registTool(*new ConsoleWindow(*Engine::getCurrentWorld()));
	toolShelf.registTool(*new AnimationConverter());
	toolShelf.registTool(*new TimelineWindow());
}