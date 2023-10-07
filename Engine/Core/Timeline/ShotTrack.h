#pragma once

#include "TimelineClipTrack.h"
#include "ShotPlayable.h"

class ENGINE_API ShotTrack : public TimelineClipTrack
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