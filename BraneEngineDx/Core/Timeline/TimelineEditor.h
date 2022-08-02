#pragma once

#include "../Editor/BaseEditor.h"
#include "TimelineEditorInfo.h"

class TimelineEditor : public BaseEditor, public TimelineEditorInfo
{
public:
	TimelineEditor() = default;
	virtual ~TimelineEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual TimelinePlayer* getPlayer() const;
	virtual void setPlayer(TimelinePlayer* player);

	virtual void onToolBarTopGUI(EditorInfo& info);
	virtual void onTimeBarGUI(EditorInfo& info);
	virtual void onClipViewGUI(EditorInfo& info, TimelineTrack& track, const Color& trackColor);
	virtual void onToolBarBottomGUI(EditorInfo& info);
	virtual void onClipScrollBarGUI(EditorInfo& info);

	virtual void onPreviewLineGUI(EditorInfo& info);

	virtual void onTimelineGUI(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);

protected:
	int defaultFrameCount = 1500;
	float unitRefSize = 10;
	float frameSize = 10;

	float trackViewWidthRadio = 0.3;

	float scrollViewHeight = 0;

	ImVec2 winPos;
	ImVec2 clipViewPos;

	Serialization* lastOpenTimeTrackSerialization = NULL;
	string newTimelineTrackName;

	struct ClipOperation
	{
		enum ClipOperationType
		{
			None, Click, Move, RightMove, LeftMove
		};

		TimelineClip* clip = NULL;
		float startFrame = 0;
		float durationFrame = 0;
		float endFrame = 0;
		ClipOperationType type = None;
	};

	ClipOperation clipOperation;

	TimelinePlayer* internalPlayer;

	float getFrameCount() const;

	float getFullViewSize() const;

	int getFramePreUnit() const;

	float getViewSize() const;
	void setViewSize(float size);

	float timeToSize(float time) const;
	float sizeToTime(float size) const;
};