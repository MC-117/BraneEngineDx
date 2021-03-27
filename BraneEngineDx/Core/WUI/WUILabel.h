#pragma once
#ifndef _WUILABEL_H_
#define _WUILABEL_H_

#include "WUIFont.h"
class WUILabel : public WUIControl
{
public:
	WUIFont font;
	WUILabel(const string& text = "");

	virtual void setText(const string& text);
	virtual void setTextColor(const Color& color);
	virtual void setTextFlag(unsigned int flag);
protected:
	Color textColor = { 250, 250, 250 };
	unsigned int textFlag = DT_SINGLELINE | DT_LEFT | DT_VCENTER;
	virtual void onPaint(HDC hdc);
	void updateText();
};

#endif // !_WUILABEL_H_
