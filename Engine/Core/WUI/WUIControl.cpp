#include "WUIControl.h"

bool WUIControl::isRegistClass = false;
unsigned int WUIControl::nextHMenuID = 1001;
WNDCLASSEX WUIControl::wndClassEx = { 0 };

WUIControl::WUIControl(HINSTANCE hIns, HWND parent)
	: hIns(hIns), parentHwnd(parent)
{
}

WUIControl::~WUIControl()
{
	if (parent) {
		for (auto b = parent->controls.begin(), e = parent->controls.end(); b != e; b++) {
			if (*b == this) {
				parent->controls.erase(b);
				break;
			}
		}
	}
	parent = NULL;
	if (hWnd) {
		SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
		DestroyWindow(hWnd);
	}
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

void WUIControl::registDefaultClass(HINSTANCE hIns, LPSTR hIcon)
{
	if (!isRegistClass) {
		wndClassEx.cbSize = sizeof(WNDCLASSEX);
		wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
		wndClassEx.lpfnWndProc = defaultWndProc;
		wndClassEx.cbClsExtra = 0;
		wndClassEx.cbWndExtra = 0;
		wndClassEx.hInstance = hIns;
		wndClassEx.hIcon = LoadIcon(wndClassEx.hInstance, hIcon);
		wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClassEx.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		wndClassEx.lpszMenuName = NULL;
		wndClassEx.lpszClassName = "BraneEngineClass";
		wndClassEx.hIconSm = LoadIcon(wndClassEx.hInstance, hIcon);
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

HWND WUIControl::getHWnd() const
{
	return hWnd;
}

void WUIControl::setStyle(DWORD winStyle)
{
	if (hWnd != NULL)
		this->winStyle = SetWindowLong(hWnd, GWL_STYLE, winStyle);
	else
		this->winStyle = winStyle;
}

void WUIControl::setExStyle(DWORD winExStyle)
{
	if (hWnd != NULL)
		this->winExStyle = SetWindowLong(hWnd, GWL_EXSTYLE, winExStyle);
	else
		this->winExStyle = winExStyle;
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
		ctrl.create();
	}
}

void WUIControl::setText(const string& text)
{
	this->text = text;
	if (hWnd != NULL) {
		int n = ::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
		wstring text_w(n, '\0');
		::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, text_w.data(), n);
		::SetWindowTextW(hWnd, text_w.c_str());
	}
}

void WUIControl::setPos(const Unit2Di& pos, bool active)
{
	this->pos = pos;
	updateFromWindowRect(active);
}

void WUIControl::setSize(const Unit2Di& size, bool active)
{
	this->size = size;
	updateFromWindowRect(active);
}

void WUIControl::setPosAndSize(const Unit2Di& pos, const Unit2Di& size, bool active)
{
	this->pos = pos;
	this->size = size;
	updateFromWindowRect(active);
}

void WUIControl::setClientPos(const Unit2Di& pos, bool active)
{
	this->clientPos = pos;
	updateFromClientRect(active);
}

void WUIControl::setClientSize(const Unit2Di& size, bool active)
{
	this->clientSize = size;
	updateFromClientRect(active);
}

void WUIControl::setClientPosAndSize(const Unit2Di& pos, const Unit2Di& size, bool active)
{
	this->clientPos = pos;
	this->clientSize = size;
	updateFromClientRect(active);
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
	while (!closing) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				closing = true;
		}
		onLoop();
	}
	ShowWindow(hWnd, SW_HIDE);
	DestroyWindow(hWnd);
	return true;
}

bool WUIControl::doModelAsync(void(*workFunc)(WUIControl& control, void* ptr), void* ptr)
{
	create();
	isAsync = true;
	show();
	if (asyncThread)
		delete asyncThread;
	asyncThread = new thread([&]() { workFunc(*this, ptr); closing = true; });
	asyncThread->detach();
	MSG msg = {};
	while (!closing) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				closing = true;
		}
		onLoop();
	}
	close();
	isAsync = false;
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
		RECT rc;
		GetWindowRect(hWnd, &rc);
		pos.x = rc.left;
		pos.y = rc.top;
		clientPos.x = LOWORD(lParam);
		clientPos.y = HIWORD(lParam);
		onMove(clientPos, clientSize);
		break;
	}
	case WM_SIZE:
		if (wParam == SIZE_MAXIMIZED) {
			pos = { 0, 0 };
			clientPos = { 0, 0 };
		}
		RECT rc;
		GetWindowRect(hWnd, &rc);
		size.x = rc.right - rc.left;
		size.y = rc.bottom - rc.top;
		clientSize.x = LOWORD(lParam);
		clientSize.y = HIWORD(lParam);
		onResize(wParam, clientSize);
		break;
	case WM_ENTERSIZEMOVE:
		isMovingOrResizing = true;
		onResizeEnter();
		return 0;
	case WM_EXITSIZEMOVE:
		isMovingOrResizing = false;
		onResizeExit();
		return 0;
	case WM_PAINT:
	{
		if (isMovingOrResizing)
			onLoop();
		onPrePaint();
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		onPaint(hdc);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_ERASEBKGND:
	{
		BOOL re = onEraseBkgnd((HDC)wParam);
		if (re)
			return 0;
		break;
	}
	case WM_NCMOUSEHOVER:
	case WM_MOUSEHOVER:
	{
		Unit2Di mpos;
		mpos.x = LOWORD(lParam);
		mpos.y = HIWORD(lParam);
		onMouseHover(wParam, mpos);
		break;
	}
	case WM_NCMOUSEMOVE:
	case WM_MOUSEMOVE:
	{
		Unit2Di mpos;
		mpos.x = LOWORD(lParam);
		mpos.y = HIWORD(lParam);
		onMouseMove(wParam, mpos);
		break;
	}
	case WM_NCMOUSELEAVE:
	case WM_MOUSELEAVE:
		onMouseLeave();
		break;
	case WM_NCLBUTTONDOWN:
	case WM_LBUTTONDOWN:
		onLBTNDown();
		break;
	case WM_NCLBUTTONUP:
	case WM_LBUTTONUP:
		onLBTNUp();
		break;
	case WM_NCLBUTTONDBLCLK:
	case WM_LBUTTONDBLCLK:
		onLBTNDBLClick();
		break;
	case WM_CLOSE:
		closing = true;
		break;
	case WM_COMMAND:
		onCommand(wParam, lParam);
		break;
	case WM_SYSCOMMAND:
		onSysCommand(wParam, lParam);
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
	int n = ::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
	wstring text_w(n, '\0');
	::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, text_w.data(), n);
	hWnd = CreateWindowExW(winExStyle, L"BraneEngineClass", text_w.c_str(), winStyle,
		pos.x, pos.y, size.x, size.y, parentHwnd, NULL, hIns, this);
	getStyle();
	getExStyle();
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

void WUIControl::onPrePaint()
{
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

void WUIControl::updateFromWindowRect(bool active)
{
	RECT rc = { 0, 0, 0, 0 };
	if (hWnd) {
		SetWindowPos(hWnd, 0, pos.x, pos.y, size.x, size.y, active ? SWP_NOOWNERZORDER : (SWP_NOOWNERZORDER | SWP_NOACTIVATE));
		GetClientRect(hWnd, &rc);
		clientPos = { rc.left, rc.top };
		clientSize = { rc.right - rc.left, rc.bottom - rc.top };
	}
	else {
		DWORD styles = getStyle();
		styles |= (!(styles & WS_CAPTION) && !(styles & WS_THICKFRAME)) ? 0 : WS_THICKFRAME;
		::AdjustWindowRectEx(&rc, styles, FALSE, getExStyle());
		clientPos = { pos.x - rc.left, pos.y - rc.top };
		clientSize = { size.x - (rc.right - rc.left), size.y - (rc.bottom - rc.top) };
	}
}

void WUIControl::updateFromClientRect(bool active)
{
	RECT rc = { 0, 0, 0, 0 };
	if (hWnd) {
		GetClientRect(hWnd, &rc);
		pos += { rc.left - clientPos.x, rc.top - clientPos.y };
		size += { rc.right - rc.left - clientSize.x, rc.bottom - rc.top - clientSize.y };
		Unit2Di offset = { rc.right - rc.left, rc.bottom - rc.top };
		SetWindowPos(hWnd, 0, pos.x, pos.y, size.x, size.y, active ? SWP_NOOWNERZORDER : (SWP_NOOWNERZORDER | SWP_NOACTIVATE));
		GetClientRect(hWnd, &rc);
		clientPos = { rc.left, rc.top };
		clientSize = { rc.right - rc.left, rc.bottom - rc.top };
	}
	else {
		DWORD styles = getStyle();
		styles |= (!(styles & WS_CAPTION) && !(styles & WS_THICKFRAME)) ? 0 : WS_THICKFRAME;
		rc = { clientPos.x, clientPos.y, clientPos.x + clientSize.x, clientPos.y + clientSize.y };
		::AdjustWindowRectEx(&rc, styles, FALSE, getExStyle());
		pos = { rc.left, rc.top };
		size = { rc.right - rc.left, rc.bottom - rc.top };
	}
}

BOOL WUIControl::onEraseBkgnd(HDC hdc)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	HBRUSH hbr = CreateSolidBrush(RGB(backColor.r * 255, backColor.g * 255, backColor.b * 255));
	FillRect(hdc, &rect, hbr);
	DeleteObject(hbr);
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
	return 0;
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

BOOL WUIControl::onMouseHover(WPARAM wParam, const Unit2Di& pos)
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

BOOL WUIControl::onMouseMove(WPARAM wParam, const Unit2Di& pos)
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
