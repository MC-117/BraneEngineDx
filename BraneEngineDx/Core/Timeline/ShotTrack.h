#pragma once

#include "TimelineClipTrack.h"
#include "ShotPlayable.h"

class ShotTrack : public TimelineClipTrack
{
public:
	Serialize(ShotTrack, TimelineClipTrack);

	ShotTrack(const string& name = "ShotTrack");
	virtual ~ShotTrack() = default;

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
};