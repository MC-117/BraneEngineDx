#pragma once
#ifndef _ESCMENU_H_
#define _ESCMENU_H_

#include "UIWindow.h"
class ENGINE_API ESCMenu : public UIWindow
{
public:
	ESCMenu(string name = "ESCMenu", bool defaultShow = false);
	virtual ~ESCMenu();

	virtual void onPreAction(GUIRenderInfo& info);
	virtual void onWindowGUI(GUIRenderInfo& info);
};

#endif // !_ESCMENU_H_
