#pragma once
#ifndef _INSPECTORWINDOW_H_
#define _INSPECTORWINDOW_H_

#include "UIWindow.h"

class Asset;

class ENGINE_API InspectorWindow : public UIWindow
{
public:
	Asset* assignAsset = NULL;

	InspectorWindow(Object& object, string name = "Inspector", bool defaultShow = false);

	virtual void onRenderWindow(GUIRenderInfo& info);
protected:
	/*void showMaterial(Material* mat, Render* render);
	void showMaterial(multimap<string, unsigned int>& meshPartNames, vector<Material*>& materials, GUI& gui);*/
};

#endif // !_INSPECTORWINDOW_H_
