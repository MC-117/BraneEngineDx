#pragma once
#include "UIWindow.h"
class CMDWindow : public UIWindow
{
public:
	CMDWindow(string name = "CMDWindow", bool defaultShow = false);
	virtual ~CMDWindow();

	virtual void onRenderWindow(GUIRenderInfo& info);
protected:
	struct History
	{
		string command;
		string output;
	};

	list<History> historyList;
	string command;
	list<string>::iterator iter;
	float historyWinSize = 0;
	float commandWinSize = 0;
	bool scroll = false;
};

