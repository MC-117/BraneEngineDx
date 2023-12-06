#pragma once
#ifndef _UICONTROL_H_
#define _UICONTROL_H_

#include "../Object.h"

class GUI;
class UIControl;
class Gizmo;

struct GUIRenderInfo
{
	Unit2Di viewSize;
	Texture* sceneBlurTex;
	SceneRenderData* sceneData;
	RenderGraph* renderGraph;
	GUI& gui;
	Camera* camera;
	Gizmo* gizmo;
};

struct GUIPostInfo
{
	UIControl* focusControl;
	GUI& gui;
};

class ENGINE_API UIControl
{
public:
	string name = "UIControl";
	Object& object;
	bool show = false;
	Events events;

	UIControl(Object& object, string name = "UIControl", bool defaultShow = false);
	virtual ~UIControl();

	virtual void onPreAction(GUIRenderInfo& info);
	virtual void onGUI(GUIRenderInfo& info);
	virtual void onPostAction(GUIPostInfo& info);
	virtual void onSceneResize(const Vector2i& size);
	virtual void onAttech(GUI& gui);

	virtual void onRender(RenderInfo& info);
};

#endif // !_UICONTROL_H_
