#pragma once
#ifndef _LOADINGUI_H_
#define _LOADINGUI_H_

#include "WUIFont.h"

class LoadingUI : public WUIControl
{
public:
	LoadingUI(const string& imgPath, HINSTANCE hIns);
	virtual ~LoadingUI();

	virtual void setText(const string& text);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	string imgPath;
	HBITMAP img = NULL;
	WUIFont font;

	void loadResource();
	virtual void onPaint(HDC hdc);
	virtual BOOL onEraseBkgnd(HDC hdc, const PAINTSTRUCT& ps);
};

#endif // !_LOADINGUI_H_
