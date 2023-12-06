#pragma once
#include "UIWindow.h"
#include "GUISurface.h"

class ENGINE_API UIViewport : public UIWindow
{
public:
	UIViewport(const string& name = "Viewport", bool defaultShow = false);

	void setCamera(Camera* camera);

	virtual void onWindowGUI(GUIRenderInfo& info);
	virtual void onPostAction(GUIPostInfo& info);
	virtual void onRender(RenderInfo& info);
protected:
	GUISurface surface;

	virtual void resize(const Vector2i& size);

	GUISurface& getTargetSurface();
};
