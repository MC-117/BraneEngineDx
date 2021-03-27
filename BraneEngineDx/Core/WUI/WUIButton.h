#pragma once
#ifndef _WUIBUTTON_H_
#define _WUIBUTTON_H_

#include "WUIFont.h"
#include "../Delegate.h"

class WUIButton : public WUIControl
{
public:
	WUIFont font;
	Delegate<WUIButton&> onHoverEvent;
	Delegate<WUIButton&> onClickedEvent;
	Delegate<WUIButton&> onDBLClickedEvent;

	WUIButton(const string& text = "");

	virtual void setText(const string& text);
	virtual void setNormalColor(const Color& color);
	virtual void setHoverColor(const Color& color);
	virtual void setPressColor(const Color& color);
	virtual Color getNormalColor() const;
	virtual Color getHoverColor() const;
	virtual Color getPressColor() const;
protected:
	bool track = false;
	bool hover = false, down = false;

	Color normalColor = { 0.126f, 0.126f, 0.126f };
	Color hoverColor = { 0.4221f, 0.4221f, 0.4221f };
	Color pressColor = { 0.2f, 0.2f, 0.2f };

	void updateColor(const Color& color, bool force = false);

	virtual void onPaint(HDC hdc);
	virtual BOOL onMouseHover(WPARAM wParam, LPARAM lParam);
	virtual BOOL onMouseMove();
	virtual BOOL onMouseLeave();
	virtual BOOL onLBTNDown();
	virtual BOOL onLBTNUp();
	virtual BOOL onLBTNDBLClick();
};

#endif // !_WUIBUTTON_H_

