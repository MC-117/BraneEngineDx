#pragma once
#ifndef _MATERIALWINDOW_H_
#define _MATERIALWINDOW_H_

#include "UIWindow.h"

class MaterialWindow : public UIWindow
{
public:
	MaterialWindow(Material* material = NULL, string name = "MaterialWindow", bool defaultShow = false);

	void setMaterial(Material& material);
	Material* getMaterial();

	virtual void onRenderWindow(GUIRenderInfo& info);
protected:
	Material* material = NULL;
};

#endif // !_MATERIALWINDOW_H_