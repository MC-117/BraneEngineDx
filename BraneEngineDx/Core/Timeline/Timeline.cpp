#include "Timeline.h"

SerializeInstance(Timeline);

Timeline::~Timeline()
{
    for (int i = 0; i < tracks.size(); i++) {
        TimelineTrack* track = tracks[i];
        delete track;
    }
    tracks.clear();
}

TimelineTrack* Timeline::createTrack(Serialization& serialization)
{
    if (!serialization.isChildOf(TimelineTrack::TimelineTrackSerialization::serialization))
        return NULL;
    Serializable* instance = serialization.instantiate(SerializationInfo());
    if (instance == NULL)
        return NULL;
    TimelineTrack* track = dynamic_cast<TimelineTrack*>(instance);
    if (track == NULL) {
        delete instance;
        return NULL;
    }
    track->index = tracks.size();
    tracks.push_back(track);
    return track;
    
}

bool Timeline::removeTrack(TimelineTrack* track)
{
    for (int i = 0; i < tracks.size(); i++) {
        if (tracks[i] == track) {
            tracks.erase(tracks.begin()+=i);
            delete track;
            apply();
            return true;
        }
    }
    return false;
}

bool Timeline::removeClip(TimelineClip* clip)
{
    for (int i = 0; i < tracks.size(); i++) {
        TimelineTrack* track = tracks[i];
        for (int j = 0; j < track->clips.size(); j++) {
            if (track->clips[j] == clip) {
                track->clips.erase(track->clips.begin() += j);
                delete clip;
                apply();
                return true;
            }
        }
    }
    return false;
}

void Timeline::apply()
{
    duration = 0;
    for (int i = 0; i < tracks.size(); i++) {
        TimelineTrack* track = tracks[i];
        track->index = i;
        track->apply();
        float end = track->startTime + track->duration;
        if (duration < end)
            duration = end;
    }
}

bool Timeline::isLoop() const
{
    return loop;
}

void Timeline::setLoop(bool value)
{
    loop = value;
}

float Timeline::getDuration() const
{
    return duration;
}

Serializable* Timeline::instantiate(const SerializationInfo& from)
{
    return new Timeline();
}

bool Timeline::deserialize(const SerializationInfo& from)
{
    Base::deserialize(from);
    from.get("name", name);
    from.get("loop", loop);
    const SerializationInfo* trackInfos = from.get("tracks");
    if (trackInfos != NULL)
        for (auto b = trackInfos->sublists.begin(), e = trackInfos->sublists.end(); b != e; b++) {
            if (b->serialization->isChildOf(TimelineTrack::TimelineTrackSerialization::serialization)) {
                TimelineTrack* track = (TimelineTrack*)b->serialization->instantiate(*b);
                track->deserialize(*b);
                tracks.push_back(track);
            }
        }
    apply();
    return true;
}

bool Timeline::serialize(SerializationInfo& to)
{
    if (!Base::serialize(to))
        return false;
    to.set("name", name);
    to.set("loop", loop);
    SerializationInfo* info = to.add("tracks");
    info->type = "TimelineTrackArray";
    for (int i = 0; i < tracks.size(); i++) {
        TimelineTrack* track = tracks[i];
        SerializationInfo* trackInfo = info->add(to_string(i));
        track->serialize(*trackInfo);
    }
    return true;
}
