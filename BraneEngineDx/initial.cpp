#include "Core\Engine.h"
#include "Core\GUI\EditorWindow.h"
#include "Core\GUI\DebugLogWindow.h"
#include "Cameras\DebugCamera.h"
#include "Actors\SkySphere.h"

void InitialWorld()
{
	//world.input.setCursorHidden(true);

	DirectLight& dirLight = *new DirectLight("DirLight");
	dirLight.setRotation(0, 45, 45);
	world += dirLight;

	SkySphere& sky = *new SkySphere();
	sky.loadDefaultTexture();
	sky.scaling(400, 400, 400);
	world += sky;

	DebugCamera& debugCamera = *new DebugCamera();
	world += debugCamera;
	world.switchCamera(debugCamera);

	EditorWindow& editorWindow = *new EditorWindow(world, Material::nullMaterial, "Editor", true);
	editorWindow.blurBackground = true;
	editorWindow.showCloseButton = false;
	world += editorWindow;
	editorWindow.show = true;

	DebugLogWindow& debugLogWindow = *new DebugLogWindow("DebugLogWindow", true);
	world += debugLogWindow;
	debugLogWindow.show = true;
}