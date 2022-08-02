#pragma once

#include "../Editor/BaseEditor.h"
#include "TimelineTrack.h"
#include "TimelineEditorInfo.h"

class TimelineTrackEditor : public BaseEditor
{
public:
	TimelineTrackEditor() = default;
	virtual ~TimelineTrackEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	Color getTrackColor() const;
	
	virtual bool onTrackGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo);
	virtual void onTrackContentGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo, const ImVec2& min_p, const ImVec2& max_p);
	virtual void onTrackInspectGUI(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);
protected:
	TimelineTrack* track;
	Color selectedTintColor = Color{ 2.0f, 2.0f, 2.0f };
	Color viewColor = Color{ 0x25, 0x25, 0x26 };
	Color trackColor = Color{ 0x03, 0x7a, 0xa7 };
};