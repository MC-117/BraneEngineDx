#include "Core/Engine.h"
#include "Tool/ToolShelf.h"
#include "Tool/AnimationConverter/AnimationConverter.h"
#include "Core/GUI/ConsoleWindow.h"
#include "Core/GUI/TextureViewer.h"

void InitialTool() {
	Texture2D& texture = *new Texture2D(string("Engine/Textures/GridInfo.png"));

	Material& pbr_mat = *getAssetByPath<Material>("Engine/Shaders/PBR.mat");
	ToolShelf& toolShelf = *new ToolShelf("ToolShelf", true);
	toolShelf.showCloseButton = false;
	world += toolShelf;

	texture.bind();
	TextureViewer::showTexture(world.renderPool.gui, texture);

	toolShelf.registTool(*new ConsoleWindow(*Engine::getCurrentWorld()));
	toolShelf.registTool(*new AnimationConverter());
}