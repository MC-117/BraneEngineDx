#include "WUIButton.h"

WUIButton::WUIButton(const string& text) : WUIControl(NULL)
{
	backColor = normalColor;
	winStyle = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON;
	this->text = text;
}

void WUIButton::setText(const string& text)
{
	if (this->text != text) {
		this->text = text;
		updateColor(backColor, true);
	}
}

void WUIButton::setNormalColor(const Color& color)
{
	normalColor = color;
	if (!hover && !down)
		updateColor(normalColor);
}

void WUIButton::setHoverColor(const Color& color)
{
	hoverColor = color;
	if (hover)
		updateColor(hoverColor);
}

void WUIButton::setPressColor(const Color& color)
{
	pressColor = color;
	if (down)
		updateColor(pressColor);
}

Color WUIButton::getNormalColor() const
{
	return normalColor;
}

Color WUIButton::getHoverColor() const
{
	return hoverColor;
}

Color WUIButton::getPressColor() const
{
	return pressColor;
}

void WUIButton::updateColor(const Color& color, bool force)
{
	if (force || backColor != color) {
		backColor = color;
		InvalidateRect(hWnd, NULL, TRUE);
		SendMessage(hWnd, WM_PAINT, 0, 0);
	}
}

void WUIButton::onPaint(HDC hdc)
{
	RECT crect;
	GetClientRect(hWnd, &crect);

	HBRUSH hbr = CreateSolidBrush(RGB(backColor.r * 255, backColor.g * 255, backColor.b * 255));
	FillRect(hdc, &crect, hbr);

	font.draw(hdc, text, crect, { 250, 250, 250 }, { 0, 0, 0, 0 }, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}

BOOL WUIButton::onMouseHover(WPARAM wParam, const Unit2Di& pos)
{
	WUIControl::onMouseHover(wParam, pos);
	hover = true;
	updateColor(hoverColor);
	mousePos = pos;
	onHoverEvent(*this);
	return 0;
}

BOOL WUIButton::onMouseMove(WPARAM wParam, const Unit2Di& pos)
{
	WUIControl::onMouseMove(wParam, pos);
	if (!track) {
		TRACKMOUSEEVENT t;
		t.cbSize = sizeof(TRACKMOUSEEVENT);
		t.dwFlags = TME_LEAVE | TME_HOVER;
		t.hwndTrack = hWnd;
		t.dwHoverTime = 5;
		TrackMouseEvent(&t);
		track = true;
	}

	if (hover)
		mouseDelta = pos - mousePos;
	else
		mouseDelta = { 0, 0 };
	mousePos = pos;
	return 0;
}

BOOL WUIButton::onMouseLeave()
{
	WUIControl::onMouseLeave();
	track = false;
	hover = false;
	down = false;
	updateColor(normalColor);
	return 0;
}

BOOL WUIButton::onLBTNDown()
{
	down = true;
	updateColor(pressColor);
	return 0;
}

BOOL WUIButton::onLBTNUp()
{
	down = false;
	updateColor(hover ? hoverColor : normalColor);
	onClickedEvent(*this);
	return WUIControl::onLBTNUp();
}

BOOL WUIButton::onLBTNDBLClick()
{
	onDBLClickedEvent(*this);
	return WUIControl::onLBTNDBLClick();
}
