#pragma once
#ifndef _WUIESCMENU_H_
#define _WUIESCMENU_H_

#include "HelpUI.h"

class WUIESCMenu : public WUIControl
{
public:
	WUILabel menuLabel;
	WUIButton playButton;
	WUIButton helpButton;
	WUIButton fullScreenButton;
	WUIButton quitButton;

	HelpUI helpUI;

	WUIESCMenu();

	void play();
	void help();
	void toggleFullscreen();
	void quit();

	WUIButton editorButton;
	void editor();
protected:
	virtual void onParentMove(const Unit2Di& pos, const Unit2Di& size);
	virtual void onParentResize(WPARAM wParam, const Unit2Di& pos, const Unit2Di& size);
	virtual void onLoop();
};

#endif // !_WUIESCMENU_H_
