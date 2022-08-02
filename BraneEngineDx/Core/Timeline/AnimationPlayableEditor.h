#pragma once

#include "TimelinePlayableEditor.h"
#include "AnimationPlayable.h"

class AnimationPlayableEditor : public TimelinePlayableEditor
{
public:
	AnimationPlayableEditor() = default;
	virtual ~AnimationPlayableEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPlayableGUI(EditorInfo& info);
protected:
	AnimationPlayable* animPlayable = NULL;
};