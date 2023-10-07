#pragma once
#ifndef _WUIFONT_H_
#define _WUIFONT_H_

#include "WUIControl.h"

class ENGINE_API WUIFont
{
public:
	WUIFont(const string& familyName = "Arial", unsigned int size = 12, unsigned int weight = FW_THIN);
	virtual ~WUIFont();

	virtual void setFont(const string& familyName, unsigned int size, unsigned int weight);

	virtual HFONT create(HDC hdc);
	virtual HFONT getHFont() const;

	virtual void draw(HDC hdc, const string& text, RECT& rect, const Color& textColor = { 255, 255, 255 },
		const Color& backColor = { 0, 0, 0, 0}, unsigned int flag = DT_SINGLELINE | DT_LEFT | DT_VCENTER);
protected:
	HFONT font;
	string familyName = "Arial";
	unsigned int size = 12;
	unsigned int weight = FW_THIN;
};

#endif // !_WUIFONT_H_
