#pragma once

#include "TimelinePlayable.h"

class TimelineTrack : public Base
{
public:
	Serialize(TimelineTrack, Base);

	string name;

	int index = -1;
	float startTime = 0;
	float duration = 0;

	TimelineTrack(const string& name = "TimelineTrack");
	virtual ~TimelineTrack();

	virtual void apply();

	virtual void onBeginPlay(const PlayInfo& info);
	virtual void onPlay(const PlayInfo& info);
	virtual void onEndPlay(const PlayInfo& info);

	virtual void onUpdate(const PlayInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};