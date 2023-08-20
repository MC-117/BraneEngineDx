#pragma once

#include "UIWindow.h"
#include "../TextureCube.h"

class TextureCubeEditorWindow : public UIWindow
{
public:
	Texture* texture = NULL;

	TextureCubeEditorWindow(string name = "TextureCubeEditor", bool defaultShow = false);

	virtual void onRenderWindow(GUIRenderInfo& info);
protected:
	Texture2D textures[6];
	TextureCube* textureCube = NULL;
};
