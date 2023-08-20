#include "WUIFont.h"
#include "../Utility/Utility.h"

WUIFont::WUIFont(const string& familyName, unsigned int size, unsigned int weight)
    : familyName(familyName), size(size), weight(weight)
{
}

WUIFont::~WUIFont()
{
	if (font != NULL)
		DeleteObject(font);
}

void WUIFont::setFont(const string& familyName, unsigned int size, unsigned int weight)
{
	this->familyName = familyName;
	this->size = size;
	this->weight = weight;
	if (font != NULL) {
		DeleteObject(font);
		font = NULL;
	}
}

HFONT WUIFont::create(HDC hdc)
{
	if (font == NULL) {
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = -MulDiv(size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		lf.lfWeight = weight;
		lstrcpy(lf.lfFaceName, familyName.c_str());
		font = CreateFontIndirect(&lf);
	}
    return font;
}

HFONT WUIFont::getHFont() const
{
    return font;
}

void WUIFont::draw(HDC hdc, const string& text, RECT& rect, const Color& textColor, const Color& backColor, unsigned int flag)
{
	HGDIOBJ oldObject = SelectObject(hdc, (HGDIOBJ)create(hdc));
	if (backColor.a == 0)
		SetBkMode(hdc, TRANSPARENT);
	else {
		SetBkMode(hdc, OPAQUE);
		SetBkColor(hdc, RGB(backColor.r * 255, backColor.g * 255, backColor.b * 255));
	}
	SetTextColor(hdc, RGB(textColor.r * 255, textColor.g * 255, textColor.b * 255));
	/*if (flag & DT_EDITCONTROL) {
		vector<string> strs = split(text, "\n");
		for (int i = 0; i < strs.size(); i++) {
			int height = DrawText(hdc, strs[i].c_str(), strs[i].size(), &rect, flag);
			OffsetRect(&rect, 0, height);
		}
	}
	else*/
		DrawText(hdc, text.c_str(), text.size(), &rect, flag);
	SelectObject(hdc, oldObject);
}
