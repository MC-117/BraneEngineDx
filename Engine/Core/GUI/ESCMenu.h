#pragma once
#ifndef _ESCMENU_H_
#define _ESCMENU_H_

#include "UIWindow.h"
class ESCMenu : public UIWindow
{
public:
	ESCMenu(string name = "ESCMenu", bool defaultShow = false);
	virtual ~ESCMenu();

	virtual void onPreAction(GUIRenderInfo& info);
	virtual void onRenderWindow(GUIRenderInfo& info);
};

#endif // !_ESCMENU_H_
