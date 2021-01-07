#include "LoadingUI.h"
#include "../../../resource.h"
#include <thread>

LoadingUI::LoadingUI(const std::string& imgPath, HINSTANCE hIns)
    : imgPath(imgPath), hIns(hIns)
{
	loadResource();
	create();
}

LoadingUI::~LoadingUI()
{
}

void LoadingUI::show()
{
	ShowWindow(hWnd, SW_SHOWDEFAULT);
}

void LoadingUI::showAsync()
{
	ShowWindowAsync(hWnd, SW_SHOWDEFAULT);
}

bool LoadingUI::doModel()
{
	MSG msg = {};
	ShowWindow(hWnd, SW_SHOW);
	BringWindowToTop(hWnd);
	UpdateWindow(hWnd);
	while (!closing && WM_QUIT != msg.message) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	ShowWindow(hWnd, SW_HIDE);
	DestroyWindow(hWnd);
	return true;
}

bool LoadingUI::doModelAsync()
{
	isAsync = true;
	ShowWindow(hWnd, SW_SHOW);
	BringWindowToTop(hWnd);
	return true;
}

void LoadingUI::close()
{
	CloseWindow(hWnd);
	closing = true;
	if (isAsync) {
		ShowWindow(hWnd, SW_HIDE);
		DestroyWindow(hWnd);
	}
}

void LoadingUI::setText(const std::string& text)
{
	this->text = text;
	updateText = true;
	InvalidateRect(hWnd, NULL, TRUE);
	if (isAsync) {
		//SendMessage(hWnd, WM_ERASEBKGND, 0, 0);
		SendMessage(hWnd, WM_PAINT, 0, 0);
	}
}

LRESULT LoadingUI::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		onPaint(hdc);
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		if (!onEraseBkgnd(hdc))
			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(hWnd, &ps);
		return 1;
	}
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void LoadingUI::loadResource()
{
	img = (HBITMAP)LoadImage(hIns, imgPath.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	BITMAP bitmap;
	GetObject(img, sizeof(bitmap), &bitmap);
	width = bitmap.bmWidth;
	height = bitmap.bmHeight;
}

bool LoadingUI::create()
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)->LRESULT {
		LoadingUI* ui = (LoadingUI*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (ui == NULL)
			return DefWindowProc(hWnd, msg, wParam, lParam);
		else
			return ui->WndProc(msg, wParam, lParam);
	};
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hIns;
	wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = LoadCursor(wcex.hInstance, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "BraneEngineLoading";
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	if (!RegisterClassEx(&wcex))
		throw std::runtime_error("Register Window Class Failed");

	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);

	hWnd = CreateWindowEx(0, "BraneEngineLoading", "BraneEngine", WS_POPUPWINDOW,
		(rect.right - width) / 2, (rect.bottom - height) / 2, width, height, NULL, NULL, wcex.hInstance, NULL);
	if (hWnd == NULL) {
		char error[100];
		sprintf_s(error, "Create Window Failed(0x%x)", GetLastError());
		throw std::runtime_error(error);
	}
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
    return hWnd != NULL;
}

void LoadingUI::onPaint(HDC hdc)
{
	if (font == NULL) {
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = -MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		lf.lfWeight = FW_THIN;
		lstrcpy(lf.lfFaceName, "Arial");
		font = CreateFontIndirect(&lf);
	}
	if (font != NULL && updateText) {
		RECT crect;
		GetClientRect(hWnd, &crect);
		RECT rectText{ 0, crect.bottom - 25, crect.right, crect.bottom };

		HDC hdcMem = CreateCompatibleDC(hdc);
		HGDIOBJ oldBitmap = SelectObject(hdcMem, img);
		BitBlt(hdc, rectText.left, rectText.top, rectText.right, rectText.bottom, hdcMem, rectText.left, rectText.top, SRCCOPY);
		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);

		HGDIOBJ default_font = SelectObject(hdc, (HGDIOBJ)font);
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(250, 250, 250));
		DrawText(hdc, text.c_str(), text.size(), &rectText, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
		SelectObject(hdc, default_font);
		updateText = false;
	}
}

BOOL LoadingUI::onEraseBkgnd(HDC hdc)
{
	if (img != NULL) {
		HDC hdcMem = CreateCompatibleDC(hdc);
		HGDIOBJ oldBitmap = SelectObject(hdcMem, img);
		BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);
	}
	return TRUE;
}
