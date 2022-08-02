#pragma once

#include "../Editor/BaseEditor.h"
#include "TimelineClip.h"

class TimelineClipEditor : public BaseEditor
{
public:
	TimelineClipEditor() = default;
	virtual ~TimelineClipEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onGUI(EditorInfo & info);
protected:
	TimelineClip* clip = NULL;
};