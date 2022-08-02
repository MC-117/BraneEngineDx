#include "WUILabel.h"
#include "../Utility/Utility.h"

WUILabel::WUILabel(const string& text) : WUIControl(NULL)
{
    backColor = { 0, 0, 0, 0 };
    winStyle = WS_VISIBLE | WS_CHILD;
    this->text = text;
}

void WUILabel::setText(const string& text)
{
	this->text = text;
    updateText();
}

void WUILabel::setTextColor(const Color& color)
{
    textColor = textColor;
    updateText();
}

void WUILabel::setTextFlag(unsigned int flag)
{
    textFlag = flag;
    updateText();
}

void WUILabel::onPaint(HDC hdc)
{
    RECT rect = { 0, 0, size.x, size.y };
    font.draw(hdc, text, rect, textColor, backColor, textFlag);
}

void WUILabel::updateText()
{
    if (hWnd != NULL) {
        InvalidateRect(hWnd, NULL, TRUE);
        SendMessage(hWnd, WM_PAINT, 0, 0);
    }
}
