#pragma once

#include "TimelineTrackEditor.h"
#include "AnimationDataTrack.h"

class AnimationDataTrackEditor : public TimelineTrackEditor
{
public:
	AnimationDataTrackEditor();
	virtual ~AnimationDataTrackEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual bool onTrackGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo);
	virtual void onTrackContentGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo, const ImVec2& min_p, const ImVec2& max_p);
	virtual void onTrackInspectGUI(EditorInfo& info);
	virtual void onClipGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo);

protected:
	AnimationDataTrack* animDataTrack;
	string newName;
	string newCurveName;
	string oldCurveName;
	string selectedCurveName;
	Curve<float, float>* selectedCurve = NULL;

	template<class T>
	struct ChannelClip
	{
		Curve<float, T>* curve;
		ImVec2 yRange;
	};

	vector<ChannelClip<float>> curveClips;
};