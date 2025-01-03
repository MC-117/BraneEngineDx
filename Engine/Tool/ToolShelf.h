#pragma once
#ifndef _TOOLSHELF_H_
#define _TOOLSHELF_H_

#include "../Core/GUI/UIWindow.h"

class ENGINE_API ToolShelf : public UIWindow
{
public:
	vector<pair<UIWindow*, bool>> tools;

	ToolShelf(string name = "ToolShelf", bool defaultShow = false);

	void registTool(UIWindow& tool);

	virtual void onWindowGUI(GUIRenderInfo& info);
};

#endif // !_TOOLSHELF_H_
