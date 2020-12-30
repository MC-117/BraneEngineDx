#include "Core\Engine.h"
#include "Core\GUI\EditorWindow.h"
#include "Core\GUI\DebugLogWindow.h"

void InitialWorld()
{
	//world.input.setCursorHidden(true);

	EditorWindow& editorWindow = *new EditorWindow(world, Material::nullMaterial, "Editor", true);
	editorWindow.blurBackground = true;
	editorWindow.showCloseButton = false;
	world += editorWindow;
	editorWindow.show = true;

	DebugLogWindow& debugLogWindow = *new DebugLogWindow("DebugLogWindow", true);
	world += debugLogWindow;
	debugLogWindow.show = true;
}