#include "TimelinePlayable.h"

SerializeInstance(TimelinePlayable);

void TimelinePlayable::onBeginPlay(const PlayInfo& info)
{
}

void TimelinePlayable::onPlay(const PlayInfo& info)
{
}

void TimelinePlayable::onEndPlay(const PlayInfo& info)
{
}

Serializable* TimelinePlayable::instantiate(const SerializationInfo& from)
{
	return new TimelinePlayable();
}

bool TimelinePlayable::deserialize(const SerializationInfo& from)
{
	return true;
}

bool TimelinePlayable::serialize(SerializationInfo& to)
{
	return Serializable::serialize(to);
}
