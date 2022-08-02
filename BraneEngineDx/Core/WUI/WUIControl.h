#pragma once
#ifndef _WUICONTROL_H_
#define _WUICONTROL_H_

#include "../Unit.h"
#include "../Delegate.h"

class WUIControl
{
public:
	WUIControl* parent = NULL;
	vector<WUIControl*> controls;

	WUIControl(HINSTANCE hIns, HWND parent = NULL);
	virtual ~WUIControl();

	static WNDCLASSEX wndClassEx;
	static LRESULT defaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static void registDefaultClass(HINSTANCE hIns);

	virtual void addControl(WUIControl& ctrl);

	virtual void setHInstance(HINSTANCE hIns);
	virtual HINSTANCE getHInstance() const;
	virtual void setStyle(DWORD winStyle);
	virtual void setExStyle(DWORD winExStyle);
	virtual DWORD getStyle();
	virtual DWORD getExStyle();

	virtual void setText(const string& text);
	virtual void setPos(const Unit2Di& pos, bool active = false);
	virtual void setSize(const Unit2Di& size, bool active = false);
	virtual void setPosAndSize(const Unit2Di& pos, const Unit2Di& size, bool active = false);
	virtual void setBackColor(const Color& bkColor);
	virtual string getText() const;
	virtual Unit2Di getPos() const;
	virtual Unit2Di getSize() const;
	virtual Color getBackColor() const;

	virtual HWND create();

	virtual bool isHide();

	virtual void show(bool active = true);
	virtual void hide();
	virtual bool doModel(bool showDefault = true);
	virtual bool doModelAsync(void(*workFunc)(WUIControl& control, void* ptr), void* ptr = NULL);
	virtual void close();
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	static unsigned int nextHMenuID;
	DWORD winStyle = WS_POPUP | WS_SYSMENU | WS_BORDER;
	DWORD winExStyle = 0;
	HINSTANCE hIns = NULL;
	HWND parentHwnd = NULL;
	HWND hWnd = NULL;
	HMENU hMenu = 0;
	WNDPROC oldWndProc = DefWindowProc;
	string text;
	Unit2Di size = { 0, 0 };
	Unit2Di pos = { 0, 0 };
	Color backColor = { 0, 0, 0 };
	bool firstShow = true, visible = false, closing = false, isAsync = false;
	thread* asyncThread = NULL;
	static bool isRegistClass;

	virtual HMENU getHMenuID();

	virtual void onCreate();
	virtual void onPaint(HDC hdc);
	virtual BOOL onEraseBkgnd(HDC hdc, const PAINTSTRUCT& ps);
	virtual BOOL onCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL onSysCommand(WPARAM wParam, LPARAM lParam);
	virtual void onParentMove(const Unit2Di& pos, const Unit2Di& size);
	virtual void onParentResize(WPARAM wParam, const Unit2Di& pos, const Unit2Di& size);
	virtual void onMove(const Unit2Di& pos, const Unit2Di& size);
	virtual void onResize(WPARAM wParam, const Unit2Di& size);
	virtual void onResizeEnter();
	virtual void onResizeExit();
	virtual BOOL onMouseHover(WPARAM wParam, const Unit2Di& pos);
	virtual BOOL onMouseMove(WPARAM wParam, const Unit2Di& pos);
	virtual BOOL onMouseLeave();
	virtual BOOL onLBTNDown();
	virtual BOOL onLBTNUp();
	virtual BOOL onLBTNDBLClick();
	virtual void onLoop();
};

#endif // !_WUICONTROL_H_
