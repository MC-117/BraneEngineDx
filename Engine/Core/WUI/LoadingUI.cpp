#include "LoadingUI.h"

LoadingUI::LoadingUI(const std::string& imgPath, HINSTANCE hIns)
    : imgPath(imgPath), WUIControl(hIns, NULL)
{
	loadResource();
}

LoadingUI::~LoadingUI()
{
}

void LoadingUI::setText(const std::string& text)
{
	this->text = text;
	InvalidateRect(hWnd, NULL, TRUE);
	SendMessage(hWnd, WM_PAINT, 0, 0);
}

LRESULT LoadingUI::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT Out = WUIControl::WndProc(msg, wParam, lParam);
	if (msg == WM_NCHITTEST) {
		Out = Out == HTCLIENT ? HTCAPTION : Out;
	}
	return Out;
}

void LoadingUI::loadResource()
{
	img = (HBITMAP)LoadImage(hIns, imgPath.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	BITMAP bitmap;
	GetObject(img, sizeof(bitmap), &bitmap);
	size.x = bitmap.bmWidth;
	size.y = bitmap.bmHeight;
	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	pos = { rect.right, rect.bottom };
	pos = (pos - size) / 2;
}

void LoadingUI::onPaint(HDC hdc)
{
	RECT crect;
	GetClientRect(hWnd, &crect);
	RECT rectText{ 0, crect.bottom - 25, crect.right, crect.bottom };

	HDC hdcMem = CreateCompatibleDC(hdc);
	HGDIOBJ oldBitmap = SelectObject(hdcMem, img);
	BitBlt(hdc, rectText.left, rectText.top, rectText.right, rectText.bottom, hdcMem, rectText.left, rectText.top, SRCCOPY);
	SelectObject(hdcMem, oldBitmap);
	DeleteDC(hdcMem);

	font.draw(hdc, text, rectText);
}

BOOL LoadingUI::onEraseBkgnd(HDC hdc)
{
	if (img != NULL) {
		HDC hdcMem = CreateCompatibleDC(hdc);
		HGDIOBJ oldBitmap = SelectObject(hdcMem, img);
		BitBlt(hdc, 0, 0, size.x, size.y, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);
	}
	return TRUE;
}
