#pragma once

#include "TimelineTrackEditor.h"
#include "AudioTrack.h"

class AudioTrackEditor : public TimelineTrackEditor
{
public:
	AudioTrackEditor();
	virtual ~AudioTrackEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onTrackContentGUI(EditorInfo & info, TimelineEditorInfo & timelineInfo, const ImVec2 & min_p, const ImVec2 & max_p);
	virtual void onTrackInspectGUI(EditorInfo & info);

protected:
	AudioTrack* audioTrack;
};