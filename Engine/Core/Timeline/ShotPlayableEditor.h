#pragma once

#include "TimelinePlayableEditor.h"
#include "ShotPlayable.h"

class ShotPlayableEditor : public TimelinePlayableEditor
{
public:
	ShotPlayableEditor() = default;
	virtual ~ShotPlayableEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPlayableGUI(EditorInfo& info);
protected:
	ShotPlayable* shotPlayable = NULL;
};