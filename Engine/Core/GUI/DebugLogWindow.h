#pragma once
#ifndef _DEBUGLOGWINDOW_H_
#define _DEBUGLOGWINDOW_H_

#include "UIWindow.h"

class ENGINE_API DebugLogWindow : public UIWindow
{
public:
	DebugLogWindow(string name = "DebugLogWindow", bool defaultShow = false);
	virtual void onWindowGUI(GUIRenderInfo& info);
protected:
	Time startTime;
};

#endif // !_DEBUGLOGWINDOW_H_
