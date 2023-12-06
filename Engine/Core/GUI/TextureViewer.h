#pragma once
#ifndef _TEXTUREVIEWER_H_
#define _TEXTUREVIEWER_H_

#include "UIWindow.h"

class ENGINE_API TextureViewer : public UIWindow
{
public:
	Texture* texture = NULL;

	TextureViewer(string name = "TextureViewer", bool defaultShow = false);
	
	void setTexture(Texture& tex);
	
	virtual void onWindowGUI(GUIRenderInfo& info);

	static void showTexture(GUI& gui, Texture& tex);
protected:
	float mipLevel = 0;
	bool invert = false;
};

#endif // !_TEXTUREVIEWER_H_
