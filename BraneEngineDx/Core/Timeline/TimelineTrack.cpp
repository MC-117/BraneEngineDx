#include "TimelineTrack.h"

SerializeInstance(TimelineTrack);

TimelineTrack::TimelineTrack(const string& name) : name(name), Base()
{
}

TimelineTrack::~TimelineTrack()
{
}

void TimelineTrack::apply()
{
}

void TimelineTrack::onBeginPlay(const PlayInfo& info)
{
}

void TimelineTrack::onPlay(const PlayInfo& info)
{
}

void TimelineTrack::onEndPlay(const PlayInfo& info)
{
}

void TimelineTrack::onUpdate(const PlayInfo& info)
{
}

Serializable* TimelineTrack::instantiate(const SerializationInfo& from)
{
    return new TimelineTrack();
}

bool TimelineTrack::deserialize(const SerializationInfo& from)
{
    Base::deserialize(from);
    from.get("name", name);
    return true;
}

bool TimelineTrack::serialize(SerializationInfo& to)
{
    Base::serialize(to);
    to.set("name", name);
    return true;
}
