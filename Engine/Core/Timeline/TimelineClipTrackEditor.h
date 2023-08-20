#pragma once

#include "TimelineTrackEditor.h"

class TimelineClipTrackEditor : public TimelineTrackEditor
{
public:
	TimelineClipTrackEditor() = default;
	virtual ~TimelineClipTrackEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual bool onTrackGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo);
	virtual void onClipGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo);
protected:
	TimelineClipTrack* clipTrack = NULL;
};