#pragma once
#include "UIWindow.h"
class ENGINE_API PhysicsDebug : public UIWindow
{
public:
	PhysicsDebug(string name = "PhysicsDebug", bool defaultShow = false);
	virtual ~PhysicsDebug();

	virtual void onRenderWindow(GUIRenderInfo& info);
};

