#include "Core\Engine.h"
#include "Core\Console.h"
#include "Core\GUI\EditorWindow.h"
#include "Core\GUI\ESCMenu.h"
#include "Core\GUI\DebugLogWindow.h"
#include "Core\GUI\MatBranchModifier.h"
#include "Cameras\DebugCamera.h"
#include "Actors\SkySphere.h"

void InitialWorld()
{
	world.input.setCursorHidden(true);

	DirectLight& dirLight = *new DirectLight("DirLight");
	dirLight.setRotation(0, -45, -45);
	world += dirLight;

	SkySphere& sky = *new SkySphere();
	sky.loadDefaultTexture();
	sky.scaling(50, 50, 50);
	world += sky;

	DebugCamera& debugCamera = *new DebugCamera();
	world += debugCamera;
	world.switchCamera(debugCamera);

	ESCMenu& escMenu = *new ESCMenu("ESCMenu");
	world += escMenu;

	EditorWindow& editorWindow = *new EditorWindow(world, Material::nullMaterial, "Editor", false);
	editorWindow.blurBackground = true;
	editorWindow.showCloseButton = false;
	world += editorWindow;

	DebugLogWindow& debugLogWindow = *new DebugLogWindow("DebugLogWindow", true);
	world += debugLogWindow;
	debugLogWindow.show = true;

	MatBranchModifier& matBranchModifier = *new MatBranchModifier("MatBranchModifier", false);
	world += matBranchModifier;

	SerializationInfo* info = getAssetByPath<SerializationInfo>(Engine::engineConfig.startMapPath);
	if (info == NULL) {
		Console::error("Can not load map '%s'", Engine::engineConfig.startMapPath.c_str());
		return;
	}

	world.loadWorld(*info);
	world.deserialize(*info);
	world.getCurrentCamera().cameraRender.renderTarget.setMultisampleLevel(Engine::engineConfig.msaa);
}