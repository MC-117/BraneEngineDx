#pragma once

#include "TimelinePlayer.h"

class TimelineEditorInfo
{
public:
	static int frameRate;
	ImDrawList* windowDrawList = NULL;
	ImDrawList* foregroundDrawList = NULL;

	float viewStartTime = 0;
	float viewEndTime = 0;
	float viewPreviewTime = 0;

	float trackViewWidth = 0;
	float clipViewWidth = 0;
	
	float barHeight = 24;

	int defaultFrameCount = 1500;
	float unitRefSize = 10;
	float frameSize = 10;

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

	Ref<Timeline> timeline;
	Ref<TimelinePlayer> player;

	float getFrameCount() const;

	float getFullViewSize() const;

	int getFramePreUnit() const;

	float getViewSize() const;
	void setViewSize(float size);

	float timeToSize(float time) const;
	float sizeToTime(float size) const;
};