#pragma once

#include "Editor.h"
#include "../Animation/AnimationClip.h"

class AnimationBaseEditor : public Editor
{
public:
	AnimationBaseEditor() = default;
	virtual ~AnimationBaseEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);
	virtual void onGUI(EditorInfo& info);
protected:
	AnimationBase* animationBase = NULL;
};