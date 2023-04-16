#pragma once

#include "../Unit.h"
#include "WUIWindow.h"
#include "../IRenderTarget.h"

class WUIImGuiWindow : public WUIWindow
{
public:
	WUIImGuiWindow();
	virtual ~WUIImGuiWindow();

	virtual void initViewport(ImGuiViewport& viewport);

	virtual HWND create();

	virtual void updateWindow();
	virtual void setWindowAlpha(float alpha);

	IDeviceSurface* getDeviceSurface() const;

	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	ImGuiViewport* viewport = NULL;
	DeviceSurfaceDesc deviceSurfaceDesc;
	IDeviceSurface* deviceSurface = NULL;
	virtual void onResize(WPARAM wParam, const Unit2Di& size);
	virtual void onResizeExit();
	virtual void onImGuiRender();
	virtual void onLoop();
};
