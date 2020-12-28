#pragma once
#ifndef _UIWINDOW_H_
#define _UIWINDOW_H_

#include "UIControl.h"
#include "../Texture2D.h"

class UIWindow : public UIControl
{
public:
	Color backgroundColor = { 255, 255, 255, 255 };
	ImGuiWindowFlags style = ImGuiWindowFlags_NoCollapse;
	bool blurBackground = true;
	bool showCloseButton = true;

	UIWindow(Object& object, string name = "UIWindow", bool defaultShow = false);

	void setBackgroundImg(Texture2D& img);
	void setBackgroundColor(Color color);
	void setStyle(ImGuiWindowFlags style);
	bool isFocus() const;
	void setFocus();

	virtual void render(GUIRenderInfo& info);
	virtual void onRenderWindow(GUIRenderInfo& info);
protected:
	Texture2D* backgroundTex = NULL;
	bool nextFocus = false;
	bool _isFocus = false;
	map<ImGuiStyleVar_, ImVec2> styleVars;
	map<ImGuiStyleVar_, float> styleFVars;
};

#endif // !_UIWINDOW_H_
