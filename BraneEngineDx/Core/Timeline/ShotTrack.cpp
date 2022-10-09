#include "ShotTrack.h"

SerializeInstance(ShotTrack);

ShotTrack::ShotTrack(const string& name) : TimelineClipTrack(name)
{
}

Serializable* ShotTrack::instantiate(const SerializationInfo& from)
{
	return new ShotTrack();
}

bool ShotTrack::deserialize(const SerializationInfo& from)
{
	return TimelineClipTrack::deserialize(from);
}

bool ShotTrack::serialize(SerializationInfo& to)
{
	return TimelineClipTrack::serialize(to);
}
