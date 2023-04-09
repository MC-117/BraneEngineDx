#pragma once
#ifndef _WUIVIEWPORT_H_
#define _WUIVIEWPORT_H_

#include "../Unit.h"
#include "WUIImGuiWindow.h"

class WUIMainWindow : public WUIImGuiWindow
{
public:
	WUIMainWindow();

	virtual void toggleFullscreen();
	virtual void hideAllUI();
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	string title;
	Time time;
	bool init = false;
	virtual void onResize(WPARAM wParam, const Unit2Di& size);
	virtual void onResizeExit();
	virtual void onLoop();
	virtual BOOL onSysCommand(WPARAM wParam, LPARAM lParam);
};

#endif // !_WUIVIEWPORT_H_
