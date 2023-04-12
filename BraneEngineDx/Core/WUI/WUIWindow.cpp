#include "WUIWindow.h"
#include <dwmapi.h>

WUIWindow::WUIWindow(HINSTANCE hIns, HWND parent) : WUIControl(hIns, parent)
{
	firstShow = false;
	winStyle = normalScreenWinStyle;
	winExStyle = WS_EX_ACCEPTFILES;
}

void WUIWindow::setHitRect(int layer, HitState state, const BoundBox& bound)
{
    hitRects[layer] = HitRect{ layer, state, bound };
}

WUIWindow::HitState WUIWindow::getHitStateInClientSpace(const Vector3f& position) const
{
    RECT windowRect;
    GetWindowRect(hWnd, &windowRect);
    if (position.x() < 0 || position.y() < 0 ||
        position.x() >(windowRect.right - windowRect.left) ||
        position.y() >(windowRect.bottom - windowRect.top))
        return Hit_None;
    for (const auto& rect : hitRects) {
        if (rect.second.bound.isInBox(position))
            return rect.second.state;
    }
    return Hit_Client;
}

bool WUIWindow::isMinimize() const
{
    if (hWnd) {
        WINDOWPLACEMENT placement;
        if (GetWindowPlacement(hWnd, &placement))
            return placement.showCmd == SW_SHOWMINIMIZED;
    }
    return false;
}

bool WUIWindow::isMaximize() const
{
    if (hWnd) {
        WINDOWPLACEMENT placement;
        if (GetWindowPlacement(hWnd, &placement))
            return placement.showCmd == SW_SHOWMAXIMIZED;
    }
    return false;
}

LRESULT WUIWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	//LRESULT lRet = 0;
	//bool fCallDWP = !DwmDefWindowProc(hWnd, msg, wParam, lParam, &lRet);
	//switch (msg) {
	//case WM_ACTIVATE:
	//{
	//	// Extend the frame into the client area.
	//	MARGINS margins;

	//	RECT rc;
	//	RECT crc;
	//	GetWindowRect(hWnd, &rc);
	//	GetClientRect(hWnd, &crc);

	//	margins.cxLeftWidth = crc.left - rc.left;
	//	margins.cxRightWidth = rc.right - crc.right;
	//	margins.cyBottomHeight = rc.bottom - crc.bottom;
	//	margins.cyTopHeight = crc.top - rc.top;

	//	DwmExtendFrameIntoClientArea(hWnd, &margins);
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

	switch (msg) {
    case WM_NCHITTEST:
    {
        POINT cursorPos;
        GetCursorPos(&cursorPos);
        ScreenToClient(hWnd, &cursorPos);
        LRESULT lResult = hitStateToLRESULT(getHitStateInClientSpace(Vector3f(cursorPos.x, cursorPos.y)));
        if (!isMaximize()) {
            // Get the mouse cursor position

            // Determine if the cursor is on the edge of the window
            const int borderWidth = 8;
            RECT windowRect;
            GetClientRect(hWnd, &windowRect);
            if (cursorPos.x < borderWidth && cursorPos.y < borderWidth)
            {
                lResult = HTTOPLEFT;
            }
            else if (cursorPos.x < borderWidth && cursorPos.y > windowRect.bottom - borderWidth)
            {
                lResult = HTBOTTOMLEFT;
            }
            else if (cursorPos.x > windowRect.right - borderWidth && cursorPos.y < borderWidth)
            {
                lResult = HTTOPRIGHT;
            }
            else if (cursorPos.x > windowRect.right - borderWidth && cursorPos.y > windowRect.bottom - borderWidth)
            {
                lResult = HTBOTTOMRIGHT;
            }
            else if (cursorPos.x < borderWidth)
            {
                lResult = HTLEFT;
            }
            else if (cursorPos.x > windowRect.right - borderWidth)
            {
                lResult = HTRIGHT;
            }
            else if (cursorPos.y < borderWidth)
            {
                lResult = HTTOP;
            }
            else if (cursorPos.y > windowRect.bottom - borderWidth)
            {
                lResult = HTBOTTOM;
            }
        }
        return lResult;
    }
    case WM_NCLBUTTONDBLCLK:
    {
        if (wParam == HTCAPTION) {
            ShowWindow(hWnd, isMaximize() ? SW_RESTORE : SW_MAXIMIZE);
            return 0;
        }
        return 1;
    }
    case WM_NCLBUTTONUP:
    {
        switch (wParam)
        {
        case HTMINBUTTON:
            ShowWindow(hWnd, SW_MINIMIZE);
            return 0;
        case HTMAXBUTTON:
            ShowWindow(hWnd, isMaximize() ? SW_RESTORE : SW_MAXIMIZE);
            return 0;
        case HTCLOSE:
            close();
            return 0;
        default:
            return 1;
        }
    }
    default:
        return WUIControl::WndProc(msg, wParam, lParam);
	}
}

bool WUIWindow::HitRect::operator<(const HitRect& r) const
{
    return layer < r.layer;
}

bool WUIWindow::HitRect::operator>(const HitRect& r) const
{
    return layer > r.layer;
}

LRESULT WUIWindow::hitStateToLRESULT(HitState state)
{
    switch (state)
    {
    case WUIWindow::Hit_None:
        return HTNOWHERE;
    case WUIWindow::Hit_Client:
        return HTCLIENT;
    case WUIWindow::Hit_Caption:
        return HTCAPTION;
    case WUIWindow::Hit_SysMenu:
        return HTSYSMENU;
    case WUIWindow::Hit_MinBox:
        return HTMINBUTTON;
    case WUIWindow::Hit_MaxBox:
        return HTMAXBUTTON;
    case WUIWindow::Hit_Close:
        return HTCLOSE;
    default:
        throw runtime_error("Unknown HitState");
    }
}

void WUIWindow::onLoop()
{
	WUIControl::onLoop();
}

BOOL WUIWindow::onSysCommand(WPARAM wParam, LPARAM lParam)
{
    switch (wParam) {
    case SC_KEYMENU:
        return 1;
    case SC_MINIMIZE:
        ShowWindow(hWnd, SW_MINIMIZE);
        return 1;
    case SC_MAXIMIZE:
        ShowWindow(hWnd, SW_MAXIMIZE);
        return 1;
    case SC_CLOSE:
        close();
        return 1;
    }
    return 0;
}
