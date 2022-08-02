#pragma once

#include "TimelineTrackEditor.h"
#include "ShotTrack.h"

class ShotTrackEditor : public TimelineTrackEditor
{
public:
	ShotTrackEditor();
	virtual ~ShotTrackEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onTrackContentGUI(EditorInfo & info, TimelineEditorInfo & timelineInfo, const ImVec2 & min_p, const ImVec2 & max_p);
	virtual void onTrackInspectGUI(EditorInfo & info);

protected:
	ShotTrack* shotTrack;
};