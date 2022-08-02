#pragma once
#include "UIWindow.h"
class PhysicsDebug : public UIWindow
{
public:
	PhysicsDebug(string name = "PhysicsDebug", bool defaultShow = false);
	virtual ~PhysicsDebug();

	virtual void onRenderWindow(GUIRenderInfo& info);
};

