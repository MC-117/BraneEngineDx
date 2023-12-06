#pragma once
#ifndef _GUI_H_
#define _GUI_H_

#include "UIControl.h"
#include "Gizmo.h"
#include "../EngineLoop/EngineLoop.h"

class Gizmo;

class ENGINE_API GUI
{
public:
	multimap<string, UIControl*> uiControls;
	map<string, void*> parameters;
	Texture2D* sceneBlurTex = NULL;
	Unit2Di viewSize = { 1280, 720 };

	GUI();
	virtual ~GUI();

	static GUI& get();
	static bool isMouseOnUI();
	static bool isAnyItemFocus();

	virtual void onGUI(RenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void onSceneResize(const Vector2i& size);

	void setSceneBlurTex(Texture2D* tex);
	void setMainControl(UIControl* uc);
	void addUIControl(UIControl& uc);
	UIControl* getUIControl(const string& name);
	vector<UIControl*> getUIControls(const string& name);
	bool destroyUIControl(const string& name);

	Events* getUIControlEvent(const string& name);

	bool showUIControl(const string& name);
	bool hideUIControl(const string& name);
	void showAllUIControl();
	void hideAllUIControl();
	bool isShowUIControl(const string& name);
	bool isAnyWindowFocus();

	void setParameter(const string& name, void* data);
	void* getParameter(const string& name);

	GUI& operator+=(UIControl& uc);
	GUI& operator+=(UIControl* uc);
protected:
	static bool mouseOnUI;
	static bool anyItemFocus;
	UIControl* mainControl = NULL;
	UIControl* focusControl = NULL;
};

#endif // !_GUI_H_
