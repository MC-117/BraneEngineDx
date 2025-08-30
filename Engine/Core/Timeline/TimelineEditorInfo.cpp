#include "TimelineEditorInfo.h"

int TimelineEditorInfo::frameRate = 30;


float TimelineEditorInfo::getFrameCount() const
{
	return timeline == NULL ? 0 : timeline->getDuration() * frameRate;
}

float TimelineEditorInfo::getFullViewSize() const
{
	float frameCount = getFrameCount();
	frameCount = frameCount == 0 ? defaultFrameCount : frameCount;
	return unitRefSize / (clipViewWidth / frameCount);
}

float TimelineEditorInfo::getViewSize() const
{
	return unitRefSize / frameSize;
}

void TimelineEditorInfo::setViewSize(float size)
{
	frameSize = unitRefSize / size;
}

float TimelineEditorInfo::timeToSize(float time) const
{
	return time * frameRate * frameSize;
}

float TimelineEditorInfo::sizeToTime(float size) const
{
	return size / frameSize / frameRate;
}

int TimelineEditorInfo::getFramePreUnit() const
{
	float viewSize = getViewSize();
	float framePerUnit = floor(viewSize);

	framePerUnit = max(framePerUnit, 1.0f);
	return framePerUnit;
}