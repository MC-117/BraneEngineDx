#pragma once
#ifndef _MATERIALWINDOW_H_
#define _MATERIALWINDOW_H_

#include "UIWindow.h"
#include "../Editor/Previewer/MaterialPreviewer.h"

class ENGINE_API MaterialWindow : public UIWindow
{
public:
	MaterialWindow(Material* material = NULL, string name = "MaterialWindow", bool defaultShow = false);

	void setMaterial(Material& material);
	Material* getMaterial();

	virtual void onWindowGUI(GUIRenderInfo& info);
	virtual void onRender(RenderInfo& info);
protected:
	Material* material = NULL;
	MaterialPreviewer previewer;
};

#endif // !_MATERIALWINDOW_H_