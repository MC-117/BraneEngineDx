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
	
	float trackHeight = 50;
	float barHeight = 24;

	Ref<Timeline> timeline;
	Ref<TimelinePlayer> player;
};