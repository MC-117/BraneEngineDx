#pragma once

#include "TimelinePlayable.h"

class TimelineClipTrack;
class ENGINE_API TimelineClip : public Base
{
	friend class TimelineClipTrack;
public:
	Serialize(TimelineClip, Base);
	float startTime = 0;
	float duration = 0;

	float blendInDuration = 0;
	float blendOutDuration = 0;

	string name;

	int index = -1;

	TimelinePlayable* playable = NULL;

	TimelineClip(const string& name = "");
	virtual ~TimelineClip();

	bool isInClip(const PlayInfo& info) const;

	virtual void onBeginPlay(const PlayInfo& info);
	virtual void onPlay(const PlayInfo& info);
	virtual void onEndPlay(const PlayInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	PlayInfo& toClipPlayInfo(const PlayInfo& trackPlatInfo, PlayInfo& clipPlayInfo);
	float getBlendWeight(const PlayInfo& info) const;
};