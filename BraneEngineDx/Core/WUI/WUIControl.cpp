#include "WUIControl.h"
#include "../../../resource.h"

bool WUIControl::isRegistClass = false;
unsigned int WUIControl::nextHMenuID = 1001;
WNDCLASSEX WUIControl::wndClassEx = { 0 };

WUIControl::WUIControl(HINSTANCE hIns, HWND parent)
	: hIns(hIns), parentHwnd(parent)
{
}

WUIControl::~WUIControl()
{
}

LRESULT WUIControl::defaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WUIControl* ui = NULL;
	if (msg == WM_CREATE) {
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		ui = (WUIControl*)cs->lpCreateParams;
		if (ui != NULL) {
			ui->hWnd = hWnd;
		}
	}
	else
		ui = (WUIControl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (ui == NULL)
		return DefWindowProc(hWnd, msg, wParam, lParam);
	else
		return ui->WndProc(msg, wParam, lParam);
}

void WUIControl::registDefaultClass(HINSTANCE hIns)
{
	if (!isRegistClass) {
		wndClassEx.cbSize = sizeof(WNDCLASSEX);
		wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
		wndClassEx.lpfnWndProc = defaultWndProc;
		wndClassEx.cbClsExtra = 0;
		wndClassEx.cbWndExtra = 0;
		wndClassEx.hInstance = hIns;
		wndClassEx.hIcon = LoadIcon(wndClassEx.hInstance, MAKEINTRESOURCE(IDI_ICON1));
		wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClassEx.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		wndClassEx.lpszMenuName = NULL;
		wndClassEx.lpszClassName = "BraneEngineClass";
		wndClassEx.hIconSm = LoadIcon(wndClassEx.hInstance, IDI_APPLICATION);
		if (!RegisterClassEx(&wndClassEx))
			throw std::runtime_error("Register Window Class Failed");
		isRegistClass = true;
	}
}

void WUIControl::setHInstance(HINSTANCE hIns)
{
	if (this->hIns == NULL)
		this->hIns = hIns;
}

HINSTANCE WUIControl::getHInstance() const
{
	return hIns;
}

void WUIControl::setStyle(DWORD winStyle)
{
	this->winStyle = winStyle;
	if (hWnd != NULL)
		SetWindowLong(hWnd, GWL_STYLE, winStyle);
}

void WUIControl::setExStyle(DWORD winExStyle)
{
	this->winExStyle = winExStyle;
	if (hWnd != NULL)
		SetWindowLong(hWnd, GWL_EXSTYLE, winExStyle);
}

DWORD WUIControl::getStyle()
{
	if (hWnd != NULL)
		winStyle = GetWindowLong(hWnd, GWL_STYLE);
	return winStyle;
}

DWORD WUIControl::getExStyle()
{
	if (hWnd != NULL)
		winExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	return winExStyle;
}

void WUIControl::addControl(WUIControl& ctrl)
{
	ctrl.parent = this;
	ctrl.winStyle |= WS_CHILD;
	controls.push_back(&ctrl);
	if (hWnd != NULL) {
		ctrl.create(), hWnd;
	}
}

void WUIControl::setText(const string& text)
{
	this->text = text;
	if (hWnd != NULL)
		SetWindowText(hWnd, text.c_str());
}

void WUIControl::setPos(const Unit2Di& pos, bool active)
{
	this->pos = pos;
	if (hWnd != NULL)
		SetWindowPos(hWnd, 0, pos.x, pos.y, size.x, size.y, active ? SWP_NOOWNERZORDER : (SWP_NOOWNERZORDER | SWP_NOACTIVATE));
}

void WUIControl::setSize(const Unit2Di& size, bool active)
{
	this->size = size;
	if (hWnd != NULL)
		SetWindowPos(hWnd, 0, pos.x, pos.y, size.x, size.y, active ? SWP_NOOWNERZORDER : (SWP_NOOWNERZORDER | SWP_NOACTIVATE));
}

void WUIControl::setPosAndSize(const Unit2Di& pos, const Unit2Di& size, bool active)
{
	this->pos = pos;
	this->size = size;
	if (hWnd != NULL)
		SetWindowPos(hWnd, 0, pos.x, pos.y, size.x, size.y, active ? SWP_NOOWNERZORDER : (SWP_NOOWNERZORDER | SWP_NOACTIVATE));
}

void WUIControl::setBackColor(const Color& bkColor)
{
	backColor = bkColor;
	if (hWnd != NULL)
		InvalidateRect(hWnd, NULL, true);
}

string WUIControl::getText() const
{
	return text;
}

Unit2Di WUIControl::getPos() const
{
	return pos;
}

Unit2Di WUIControl::getSize() const
{
	return size;
}

Color WUIControl::getBackColor() const
{
	return backColor;
}

bool WUIControl::isHide()
{
	return !visible;
}

void WUIControl::show(bool active)
{
	if (hWnd != NULL) {
		ShowWindow(hWnd, active ? SW_SHOW : SW_SHOWNOACTIVATE);
		if (active)
			BringWindowToTop(hWnd);
		UpdateWindow(hWnd);
	}
	visible = true;
}

void WUIControl::hide()
{
	if (hWnd != NULL) {
		ShowWindow(hWnd, SW_HIDE);
	}
	visible = false;
}

bool WUIControl::doModel(bool showDefault)
{
	create();
	MSG msg = {};
	if (showDefault) {
		show();
	}
	while (!closing && WM_QUIT != msg.message) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
			onLoop();
	}
	ShowWindow(hWnd, SW_HIDE);
	DestroyWindow(hWnd);
	return true;
}

bool WUIControl::doModelAsync()
{
	create();
	isAsync = true;
	show();
	for (int i = 0; i < controls.size(); i++) {
		WUIControl* con = controls[i];
		con->doModelAsync();
	}
	return true;
}

void WUIControl::close()
{
	CloseWindow(hWnd);
	closing = true;
	if (isAsync) {
		ShowWindow(hWnd, SW_HIDE);
		DestroyWindow(hWnd);
	}
}

LRESULT WUIControl::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		onCreate();
		break;
	case WM_MOVE:
	{
		pos.x = LOWORD(lParam);
		pos.y = HIWORD(lParam);
		onMove(pos, size);
		break;
	}
	case WM_SIZE:
		if (wParam == SIZE_MAXIMIZED)
			pos = { 0, 0 };
		size.x = LOWORD(lParam);
		size.y = HIWORD(lParam);
		onResize(wParam, size);
		break;
	case WM_ENTERSIZEMOVE:
		onResizeEnter();
		break;
	case WM_EXITSIZEMOVE:
		onResizeExit();
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		onPaint(hdc);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_ERASEBKGND:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		BOOL re = onEraseBkgnd(hdc, ps);
		EndPaint(hWnd, &ps);
		if (re)
			return 0;
		break;
	}
	case WM_MOUSEHOVER:
		if (onMouseHover(wParam, lParam))
			return 0;
		break;
	case WM_MOUSEMOVE:
		if (onMouseMove())
			return 0;
		break;
	case WM_MOUSELEAVE:
		if (onMouseLeave())
			return 0;
		break;
	case WM_LBUTTONDOWN:
		if (onLBTNDown())
			return 0;
		break;
	case WM_LBUTTONUP:
		if (onLBTNUp())
			return 0;
		break;
	case WM_LBUTTONDBLCLK:
		if (onLBTNDBLClick())
			return 0;
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		if (onCommand(wParam, lParam))
			return 0;
		break;
	case WM_SYSCOMMAND:
		if (onSysCommand(wParam, lParam))
			return 0;
		break;
	}
	return oldWndProc(hWnd, msg, wParam, lParam);
}

HWND WUIControl::create()
{
	if (!isRegistClass)
		throw runtime_error("Not regist default window class");
	if (hWnd != NULL)
		return hWnd;
	if (parent != NULL) {
		parentHwnd = parent->create();
		hIns = parent->hIns;
	}
	hWnd = CreateWindowEx(winExStyle, "BraneEngineClass", text.c_str(), winStyle,
		pos.x, pos.y, size.x, size.y, parentHwnd, NULL, hIns, this);
	if (hWnd == NULL) {
		char error[100];
		sprintf_s(error, "Create Window Failed(0x%x)", GetLastError());
		throw std::runtime_error(error);
	}
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
	/*SetParent(hWnd, parentHwnd);
	UpdateWindow(parentHwnd);*/
	return hWnd;
}

void WUIControl::onCreate()
{
	for (int i = 0; i < controls.size(); i++)
		controls[i]->create();
}

void WUIControl::onPaint(HDC hdc)
{
}

HMENU WUIControl::getHMenuID()
{
	if (hMenu == NULL) {
		hMenu = (HMENU)nextHMenuID;
		nextHMenuID++;
	}
	return hMenu;
}

BOOL WUIControl::onEraseBkgnd(HDC hdc, const PAINTSTRUCT& ps)
{
	HBRUSH hbr = CreateSolidBrush(RGB(backColor.r * 255, backColor.g * 255, backColor.b * 255));
	FillRect(hdc, &ps.rcPaint, hbr);
	return 1;
}

BOOL WUIControl::onCommand(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

BOOL WUIControl::onSysCommand(WPARAM wParam, LPARAM lParam)
{
	if ((wParam & 0xfff0) == SC_KEYMENU)
		return 1;
}

void WUIControl::onParentMove(const Unit2Di& pos, const Unit2Di& size)
{
}

void WUIControl::onParentResize(WPARAM wParam, const Unit2Di& pos, const Unit2Di& size)
{
}

void WUIControl::onMove(const Unit2Di& pos, const Unit2Di& size)
{
	for (int i = 0; i < controls.size(); i++)
		controls[i]->onParentMove(pos, size);
}

void WUIControl::onResize(WPARAM wParam, const Unit2Di& size)
{
	for (int i = 0; i < controls.size(); i++)
		controls[i]->onParentResize(wParam, pos, size);
}

void WUIControl::onResizeEnter()
{
}

void WUIControl::onResizeExit()
{
}

BOOL WUIControl::onMouseHover(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

BOOL WUIControl::onLBTNDown()
{
	return 0;
}

BOOL WUIControl::onLBTNUp()
{
	return 0;
}

BOOL WUIControl::onMouseMove()
{
	return 0;
}

BOOL WUIControl::onMouseLeave()
{
	return 0;
}

BOOL WUIControl::onLBTNDBLClick()
{
	return 0;
}

void WUIControl::onLoop()
{
	if (firstShow) {
		show();
		firstShow = false;
	}
	for (int i = 0; i < controls.size(); i++)
		controls[i]->onLoop();
}
