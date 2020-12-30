#pragma once
#ifndef _TEXTUREVIEWER_H_
#define _TEXTUREVIEWER_H_

#include "UIWindow.h"

class TextureViewer : public UIWindow
{
public:
	Texture* texture = NULL;

	TextureViewer(string name = "TextureViewer", bool defaultShow = false);
	
	void setTexture(Texture& tex);
	
	virtual void onRenderWindow(GUIRenderInfo& info);

	static void showTexture(GUI& gui, Texture& tex);
};

#endif // !_TEXTUREVIEWER_H_
