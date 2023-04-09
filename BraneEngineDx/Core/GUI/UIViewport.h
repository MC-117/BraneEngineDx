#pragma once
#include "UIWindow.h"

class UIViewport : public UIWindow
{
public:
	UIViewport(const string& name = "Viewport", bool defaultShow = false);

	void setCamera(Camera* camera);

	virtual void onRenderWindow(GUIRenderInfo& info);
protected:
	Ref<Camera> camera;
	Vector2i size;

	virtual void resize(int width, int height);
};