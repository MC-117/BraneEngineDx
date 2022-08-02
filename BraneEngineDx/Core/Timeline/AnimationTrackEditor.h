#pragma once

#include "TimelineTrackEditor.h"
#include "AnimationTrack.h"

class AnimationTrackEditor : public TimelineTrackEditor
{
public:
	AnimationTrackEditor();
	virtual ~AnimationTrackEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onTrackContentGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo, const ImVec2& min_p, const ImVec2& max_p);
	virtual void onTrackInspectGUI(EditorInfo& info);

protected:
	AnimationTrack* animTrack;
};