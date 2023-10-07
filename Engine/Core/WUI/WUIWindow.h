#pragma once

#include "../Unit.h"
#include "../Utility/Boundings.h"
#include "WUIControl.h"

class ENGINE_API WUIWindow : public WUIControl
{
public:
	enum HitState
	{
		Hit_None, Hit_Client, Hit_Caption, Hit_SysMenu, Hit_MinBox, Hit_MaxBox, Hit_Close
	};

	WUIWindow(HINSTANCE hIns = NULL, HWND parent = NULL);

	void setHitRect(int layer, HitState state, const BoundBox& bound);
	HitState getHitStateInClientSpace(const Vector3f& position) const;

	bool isMinimize() const;
	bool isMaximize() const;

	virtual HWND create();

	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	struct HitRect
	{
		int layer;
		HitState state;
		BoundBox bound;

		bool operator< (const HitRect& r) const;
		bool operator> (const HitRect& r) const;
	};
	map<int, HitRect, greater<int>> hitRects;
	DWORD normalScreenWinStyle = WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
	DWORD fullScreenWinStyle = WS_OVERLAPPED | WS_SYSMENU;
	int resizeBorderWidth = 0;

	virtual void updateFromWindowRect(bool active);
	virtual void updateFromClientRect(bool active);

	static LRESULT hitStateToLRESULT(HitState state);
	virtual void onLoop();
	virtual BOOL onSysCommand(WPARAM wParam, LPARAM lParam);
};
