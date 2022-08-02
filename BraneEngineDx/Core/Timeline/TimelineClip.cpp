#include "TimelineClip.h"

SerializeInstance(TimelineClip);

TimelineClip::TimelineClip(const string& name) : name(name), Base()
{
}

TimelineClip::~TimelineClip()
{
    if (playable != NULL) {
        delete playable;
        playable = NULL;
    }
}

bool TimelineClip::isInClip(const PlayInfo& info) const
{
    return startTime <= info.currentTime &&
        info.currentTime < (startTime + duration);
}

void TimelineClip::onBeginPlay(const PlayInfo& info)
{
    if (playable == NULL)
        return;
    PlayInfo clipPlayInfo;
    playable->onBeginPlay(toClipPlayInfo(info, clipPlayInfo));
}

void TimelineClip::onPlay(const PlayInfo& info)
{
    if (playable == NULL)
        return;
    PlayInfo clipPlayInfo;
    playable->onPlay(toClipPlayInfo(info, clipPlayInfo));
}

void TimelineClip::onEndPlay(const PlayInfo& info)
{
    if (playable == NULL)
        return;
    PlayInfo clipPlayInfo;
    playable->onEndPlay(toClipPlayInfo(info, clipPlayInfo));
}

Serializable* TimelineClip::instantiate(const SerializationInfo& from)
{
    return new TimelineClip();
}

bool TimelineClip::deserialize(const SerializationInfo& from)
{
    Base::deserialize(from);
    from.get("name", name);
    from.get("startTime", startTime);
    from.get("duration", duration);
    const SerializationInfo* playableInfo = from.get("playable");
    if (playableInfo != NULL &&
        playableInfo->serialization != NULL &&
        playableInfo->serialization->isChildOf(
            TimelinePlayable::TimelinePlayableSerialization::serialization)) {
        playable = (TimelinePlayable*)playableInfo->serialization->instantiate(*playableInfo);
        playable->deserialize(*playableInfo);
    }
    else {
        playable = NULL;
    }
    return true;
}

bool TimelineClip::serialize(SerializationInfo& to)
{
    Base::serialize(to);
    to.set("name", name);
    to.set("startTime", startTime);
    to.set("duration", duration);
    if (playable != NULL)
        to.set("playable", *playable);
    return true;
}

PlayInfo& TimelineClip::toClipPlayInfo(const PlayInfo& trackPlatInfo, PlayInfo& clipPlayInfo)
{
    clipPlayInfo.deltaTime = trackPlatInfo.deltaTime;
    clipPlayInfo.currentTime = trackPlatInfo.currentTime - startTime;
    clipPlayInfo.normalizedTime = clipPlayInfo.currentTime / duration;
    clipPlayInfo.startTime = startTime;
    clipPlayInfo.endTime = startTime + duration;
    return clipPlayInfo;
}

float TimelineClip::getBlendWeight(const PlayInfo& info) const
{
    if (isInClip(info)) {
        float blendInTime = startTime + blendInDuration;
        float endTime = startTime + duration;
        float blendOutTime = endTime - blendOutDuration;
        if (blendInDuration != 0 && info.currentTime < blendInTime)
            return (info.currentTime - startTime) / blendInDuration;
        if (blendOutDuration != 0 && info.currentTime >= blendOutTime)
            return (endTime - info.currentTime) / blendOutDuration;
        return 1;
    }
    return 0;
}
