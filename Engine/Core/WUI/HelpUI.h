#pragma once
#ifndef _HELPUI_H_
#define _HELPUI_H_

#include "WUILabel.h"
#include "WUIButton.h"

class HelpUI : public WUIControl
{
public:
	WUIButton closeButton;
	WUILabel titleLabel;
	WUILabel textLabel;

	HelpUI();
protected:
	virtual BOOL onEraseBkgnd(HDC hdc);
};

#endif // !_HELPUI_H_
