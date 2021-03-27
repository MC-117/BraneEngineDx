#pragma once
#ifndef _WUIVIEWPORT_H_
#define _WUIVIEWPORT_H_

#include "../Unit.h"
#include "WUIControl.h"

class WUIViewport : public WUIControl
{
public:
	WUIViewport(HINSTANCE hIns = NULL);

	virtual void toggleFullscreen();
	virtual void hideAllUI();
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	string title;
	DWORD normalScreenWinStyle = WS_OVERLAPPEDWINDOW;// WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX;
	DWORD fullScreenWinStyle = WS_OVERLAPPED | WS_SYSMENU;
	Time time;
	bool init = false;
	virtual void onResize(WPARAM wParam, const Unit2Di& size);
	virtual void onResizeExit();
	virtual void onLoop();
	virtual BOOL onSysCommand(WPARAM wParam, LPARAM lParam);
};

#endif // !_WUIVIEWPORT_H_
