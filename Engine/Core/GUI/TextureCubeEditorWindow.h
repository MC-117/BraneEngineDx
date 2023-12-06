#pragma once

#include "UIWindow.h"
#include "../TextureCube.h"

class ENGINE_API TextureCubeEditorWindow : public UIWindow
{
public:
	Texture* texture = NULL;

	TextureCubeEditorWindow(string name = "TextureCubeEditor", bool defaultShow = false);

	virtual void onWindowGUI(GUIRenderInfo& info);
protected:
	Texture2D textures[6];
	TextureCube* textureCube = NULL;
};
