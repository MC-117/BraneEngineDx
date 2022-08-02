#pragma once

#include "../Editor/Editor.h"
#include "TimelinePlayable.h"

class TimelinePlayableEditor : public Editor
{
public:
	TimelinePlayableEditor() = default;
	virtual ~TimelinePlayableEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPlayableGUI(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);
protected:
	TimelinePlayable* playable = NULL;
};