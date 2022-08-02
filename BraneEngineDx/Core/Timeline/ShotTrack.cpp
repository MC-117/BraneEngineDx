#include "ShotTrack.h"

SerializeInstance(ShotTrack);

ShotTrack::ShotTrack(const string& name) : TimelineTrack(name)
{
}

Serializable* ShotTrack::instantiate(const SerializationInfo& from)
{
	return new ShotTrack();
}

bool ShotTrack::deserialize(const SerializationInfo& from)
{
	return TimelineTrack::deserialize(from);
}

bool ShotTrack::serialize(SerializationInfo& to)
{
	return TimelineTrack::serialize(to);
}
