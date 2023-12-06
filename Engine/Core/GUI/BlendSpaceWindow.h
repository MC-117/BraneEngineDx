#pragma once
#ifndef _BLENDSPACEWINDOW_H_
#define _BLENDSPACEWINDOW_H_

#include "UIWindow.h"
#include "../Animation/AnimationClip.h"

struct BlendSpaceGridContext
{
	enum OperationType
	{
		Add, PreviewPosChange, Drag
	};

	vector<ImVec2> points;
	vector<pair<unsigned int, float>> weights;
	ImVec2 previewPos = { 0, 0 };
	ImVec2 dragPosReserve;
	ImVec2 dragPos;
	ImVec2 padding = { 5, 5 };
	unsigned int highlightIndex = -1, dragIndex = -1;
	unsigned int xStep = 1, yStep = 0;
	unsigned int pointSize = 5;
	unsigned int pointBorder = 2;
	unsigned int pointHitRadius = 8;

};

class ENGINE_API BlendSpaceWindow : public UIWindow
{
public:
	BlendSpaceWindow(string name = "BlendSpaceWindow", bool defaultShow = false);

	void setBlendSpaceAnimation(BlendSpaceAnimation& animation);

	virtual void onWindowGUI(GUIRenderInfo& info);
protected:
	BlendSpaceGridContext context;
	BlendSpaceAnimation* blendSpace = NULL;

	void drawBlendSpace(GUIRenderInfo & info);
};

#endif // !_BLENDSPACEWINDOW_H_
