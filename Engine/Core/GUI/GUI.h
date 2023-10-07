#pragma once
#ifndef _GUI_H_
#define _GUI_H_

#include "UIControl.h"
#include "Gizmo.h"
#include "Core/EngineLoop.h"

class Gizmo;

class ENGINE_API GUI
{
public:
	multimap<string, UIControl*> uiControls;
	map<string, void*> parameters;
	Texture2D* sceneBlurTex = NULL;
	Unit2Di viewSize = { 1280, 720 };
	Gizmo gizmo;

	GUI();
	virtual ~GUI();

	static bool isMouseOnUI();
	static bool isAnyItemFocus();

	virtual void onGUI(RenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void onSceneResize(Unit2Di size);

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

class ENGINE_API GUIEngineLoop : public EngineLoop
{
public:
	GUIEngineLoop(GUI& gui);
	
	virtual bool willQuit();
	
	virtual void init();
	virtual void loop(float deltaTime);
	virtual void release();
protected:
	GUI& gui;
};

#endif // !_GUI_H_
