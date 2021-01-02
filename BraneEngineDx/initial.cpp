#include "Core\Engine.h"
#include "Core\GUI\EditorWindow.h"
#include "Core\GUI\DebugLogWindow.h"

void InitialWorld()
{
	//world.input.setCursorHidden(true);

	DirectLight& dirLight = *new DirectLight("DirLight");
	dirLight.setRotation(0, 45, 45);
	world += dirLight;

	EditorWindow& editorWindow = *new EditorWindow(world, Material::nullMaterial, "Editor", true);
	editorWindow.blurBackground = true;
	editorWindow.showCloseButton = false;
	world += editorWindow;
	editorWindow.show = true;

	DebugLogWindow& debugLogWindow = *new DebugLogWindow("DebugLogWindow", true);
	world += debugLogWindow;
	debugLogWindow.show = true;
}