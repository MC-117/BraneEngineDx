#pragma once

#include "../Unit.h"
#include "WUIWindow.h"
#include "../IRenderTarget.h"
#include "../Utility/Parallel.h"

class ENGINE_API WUIImGuiWindow : public WUIWindow
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
	ImGuiViewport* viewport;
	DeviceSurfaceDesc deviceSurfaceDesc;
	IDeviceSurface* deviceSurface;
	WaitHandle renderWaitHandle;
	virtual void onResize(WPARAM wParam, const Unit2Di& size);
	virtual void onResizeExit();
	virtual void onImGuiRender();
	virtual void onLoop();
	virtual void onClose();
};
