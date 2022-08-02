#pragma once

#include "TimelineTrack.h"
#include "ShotPlayable.h"

class ShotTrack : public TimelineTrack
{
public:
	Serialize(ShotTrack, TimelineTrack);

	ShotTrack(const string& name = "ShotTrack");
	virtual ~ShotTrack() = default;

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
};