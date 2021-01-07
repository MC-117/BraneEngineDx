#pragma once
#ifndef _LOADINGUI_H_
#define _LOADINGUI_H_

#include <windows.h>
#include <iostream>

class LoadingUI
{
public:
	LoadingUI(const std::string& imgPath, HINSTANCE hIns);
	virtual ~LoadingUI();

	void show();
	void showAsync();
	bool doModel();
	bool doModelAsync();
	void close();
	void setText(const std::string& text);
	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	std::string imgPath;
	std::string text;
	bool updateText = false;
	bool closing = false, isAsync = false;
	HINSTANCE hIns = NULL;
	HWND hWnd = NULL;
	HBITMAP img = NULL;
	HFONT font = NULL;
	unsigned int width = 0, height = 0;

	void loadResource();
	bool create();
	virtual void onPaint(HDC hdc);
	virtual BOOL onEraseBkgnd(HDC hdc);
};

#endif // !_LOADINGUI_H_
