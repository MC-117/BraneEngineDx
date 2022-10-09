#pragma once

#include "UIWindow.h"
#include "../Animation/AnimationClip.h"

class AnimationDataWindow : public UIWindow
{
public:
	AnimationDataWindow(string name = "AnimationDataWindow", bool defaultShow = false);

	void setAnimationData(AnimationClipData* data);

	virtual void onRenderWindow(GUIRenderInfo& info);

	static void showAnimationData(GUI& gui, AnimationClipData* data);
protected:
	AnimationClipData* data = NULL;
	string newName;
	string newCurveName;
	string oldCurveName;
	string selectedCurveName;
	Curve<float, float>* selectedCurve = NULL;
	float inspectViewWidthRadio = 0.2f;
};