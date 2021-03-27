#include "WUIViewPort.h"
#include "../Engine.h"
#include "../Console.h"

WUIViewport::WUIViewport(HINSTANCE hIns) : WUIControl(hIns)
{
	firstShow = false;
	winStyle = normalScreenWinStyle;
	winExStyle = WS_EX_ACCEPTFILES;
}

void WUIViewport::toggleFullscreen()
{
	Engine::windowContext.fullscreen = !Engine::windowContext.fullscreen;
	winStyle = Engine::windowContext.fullscreen ? fullScreenWinStyle : normalScreenWinStyle;
	if (hWnd != NULL)
		SetWindowLong(hWnd, GWL_STYLE, winStyle);
	if (Engine::windowContext.fullscreen) {
		Engine::windowContext.screenPos = getPos();
		setPosAndSize({ 0, 0 }, Engine::windowContext.fullscreenSize);
	}
	else {
		RECT rc = { 0, 0, (int)Engine::engineConfig.screenWidth, (int)Engine::engineConfig.screenHeight };
		AdjustWindowRect(&rc, winStyle, FALSE);
		setPosAndSize(Engine::windowContext.screenPos, { rc.right - rc.left, rc.bottom - rc.top });
	}
	if (hWnd != NULL) {
		ShowWindow(hWnd, SW_RESTORE);
	}
	/*----- Vendor toggle fullscreen -----*/
	{
		if (!VendorManager::getInstance().getVendor().toggleFullscreen(Engine::engineConfig, Engine::windowContext, Engine::windowContext.fullscreen))
			throw runtime_error("Vendor toggle fullscreen failed");
	}
}

void WUIViewport::hideAllUI()
{
	for (int i = 0; i < controls.size(); i++)
		controls[i]->hide();
}

LRESULT WUIViewport::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	/*----- Vendor envoke WndProc -----*/
	{
		VendorManager::getInstance().getVendor().wndProcFunc(hWnd, msg, wParam, lParam);
	}

	return WUIControl::WndProc(msg, wParam, lParam);
}

void WUIViewport::onResize(WPARAM wParam, const Unit2Di& size)
{
	if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
		Engine::setViewportSize(size);
	WUIControl::onResize(wParam, size);
}

void WUIViewport::onResizeExit()
{
	Engine::setViewportSize(size);
}

void WUIViewport::onLoop()
{
	Time lastTime = time;
	time = Time::now();
	Time deltaTime = time - lastTime;
	Engine::mainLoop(deltaTime.toMillisecond() * 0.001);
	if (!init) {
		ShowWindow(hWnd, SW_SHOW);
		SetForegroundWindow(hWnd);
		BringWindowToTop(hWnd);
		SetActiveWindow(hWnd);
		init = true;
		visible = true;
		time = Time::now();
	}

	WUIControl::onLoop();
	if (title.empty())
		title = text;
	setText(title + " | FPS: " + to_string(1000 / deltaTime.toMillisecond()));
	Timer timer;
	timer.setIntervalMode(true);
	timer.record("DeltaTime", time);
	Console::getTimer("DeltaTime") = timer;
}

BOOL WUIViewport::onSysCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == SC_KEYMENU)
		return 1;
	return 0;
}
