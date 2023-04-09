#include "WUIMainWindow.h"
#include "../Engine.h"
#include "../Console.h"
#include <dwmapi.h>

WUIMainWindow::WUIMainWindow() : WUIImGuiWindow()
{
}

void WUIMainWindow::toggleFullscreen()
{
	Engine::windowContext.fullscreen = !Engine::windowContext.fullscreen;
	winStyle = Engine::windowContext.fullscreen ? fullScreenWinStyle : normalScreenWinStyle;
	if (hWnd != NULL)
		winStyle = SetWindowLong(hWnd, GWL_STYLE, winStyle);
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

void WUIMainWindow::hideAllUI()
{
	for (int i = 0; i < controls.size(); i++)
		controls[i]->hide();
}

LRESULT WUIMainWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{

	//switch (msg) {
	//case WM_ACTIVATE:
	//{
	//	// Extend the frame into the client area.
	//	MARGINS margins;

	//	int height = (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION) +
	//		GetSystemMetrics(SM_CXPADDEDBORDER));

	//	margins.cxLeftWidth = 0;
	//	margins.cxRightWidth = 0;
	//	margins.cyBottomHeight = 0;
	//	margins.cyTopHeight = height;

	//	DwmExtendFrameIntoClientArea(hWnd, &margins);

	//	return 0;
	//}
	//case WM_CREATE:
	//{
	//	RECT rcClient;
	//	GetWindowRect(hWnd, &rcClient);

	//	// Inform the application of the frame change.
	//	SetWindowPos(hWnd,
	//		NULL,
	//		rcClient.left, rcClient.top,
	//		rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
	//		SWP_FRAMECHANGED);
	//	break;
	//}
	//}

	return WUIImGuiWindow::WndProc(msg, wParam, lParam);
}

void WUIMainWindow::onResize(WPARAM wParam, const Unit2Di& size)
{
	Unit2Di safeSize = { max(1, size.x), max(1, size.y) };
	Engine::setViewportSize(safeSize);
	WUIImGuiWindow::onResize(wParam, safeSize);
}

void WUIMainWindow::onResizeExit()
{
	Engine::setViewportSize(size);
}

void WUIMainWindow::onLoop()
{
	if (closing)
		return;
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

	WUIImGuiWindow::onLoop();
	if (title.empty())
		title = text;
	setText(title + " | FPS: " + to_string(1000 / deltaTime.toMillisecond()));
	Timer timer;
	timer.setIntervalMode(true);
	timer.record("DeltaTime", time);
	Console::getTimer("DeltaTime") = timer;
}

BOOL WUIMainWindow::onSysCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == SC_KEYMENU)
		return 1;
	return 0;
}
