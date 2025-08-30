#include "TimelinePlayer.h"

SerializeInstance(TimelinePlayer);

TimelinePlayer::TimelinePlayer(const string& name) : Object(name)
{
}

TimelinePlayer::~TimelinePlayer()
{
}

void TimelinePlayer::setTimeline(Timeline* data)
{
    timeline = data;
    if (timeline == NULL) {
        playing = false;
        playInfo.currentTime = 0;
        playInfo.endTime = 0;
        playInfo.deltaTime = 0;
        playInfo.startTime = 0;
        playInfo.normalizedTime = 0;
    }
}

Timeline* TimelinePlayer::getTimeline() const
{
    return timeline;
}


bool TimelinePlayer::isPlay() const
{
    return playing;
}

bool TimelinePlayer::isLoop() const
{
    return loop || (timeline != NULL && timeline->isLoop());
}

void TimelinePlayer::setLoop(bool value)
{
    loop = value;
}

void TimelinePlayer::tick(float deltaTime)
{
    Object::tick(deltaTime);
    if (timeline == NULL)
        return;
    if (!playing)
        return;
    float lastTime = playInfo.currentTime;
    float duration = timeline->getDuration();
    playInfo.deltaTime = deltaTime;
    playInfo.currentTime += deltaTime;
    playInfo.normalizedTime = playInfo.currentTime / duration;
    bool overflow = playInfo.normalizedTime >= 1;
    bool _isLoop = isLoop();
    if (overflow) {
        if (_isLoop) {
            playInfo.currentTime = playInfo.currentTime - duration;
            playInfo.normalizedTime = playInfo.currentTime / duration;
            lastTime = 0;
        }
        else {
            playing = false;
            playInfo.currentTime = duration;
            playInfo.normalizedTime = 1;
        }
    }
    for (int i = 0; i < timeline->tracks.size(); i++) {
        TimelineTrack* track = timeline->tracks[i];
        if (overflow) {
            track->onEndPlay(playInfo);
            if (!_isLoop)
                continue;
        }
        float endTime = track->startTime + track->duration;
        bool inTrack = track->startTime <= playInfo.currentTime &&
            playInfo.currentTime < endTime;
        if (inTrack) {
            if (lastTime < track->startTime && playInfo.currentTime >= track->startTime)
                track->onBeginPlay(playInfo);
            else
                track->onPlay(playInfo);
        }
        else if (lastTime < endTime && playInfo.currentTime >= endTime) {
            track->onEndPlay(playInfo);
        }
    }
}

float TimelinePlayer::getTime() const
{
    return playInfo.currentTime;
}

float TimelinePlayer::getNormalizedTime() const
{
    return playInfo.normalizedTime;
}

void TimelinePlayer::setTime(float time)
{
    if (timeline == NULL)
        return;
    float duration = timeline->getDuration();
    time = min(time, duration);
    time = max(time, 0.0f);
    playInfo.currentTime = time;
    playInfo.normalizedTime = playInfo.currentTime / duration;
    playInfo.deltaTime = 0;

    for (int i = 0; i < timeline->tracks.size(); i++) {
        TimelineTrack* track = timeline->tracks[i];
        track->onEndPlay(playInfo);
        track->onBeginPlay(playInfo);
        if (!playing)
            track->onEndPlay(playInfo);
    }
}

void TimelinePlayer::setNormalizedTime(float time)
{
    float duration = timeline->getDuration();
    setTime(time * duration);
}

void TimelinePlayer::play()
{
    if (timeline == NULL)
        return;
    if (playing)
        return;
    float duration = timeline->getDuration();
    playing = true;
    playInfo.startTime = playInfo.currentTime;
    playInfo.endTime = duration;
    playInfo.normalizedTime = playInfo.currentTime / duration;
    playInfo.deltaTime = 0;
}

void TimelinePlayer::pause()
{
    if (timeline == NULL)
        return;
    playing = false;
    playInfo.deltaTime = 0;
    for (int i = 0; i < timeline->tracks.size(); i++) {
        TimelineTrack* track = timeline->tracks[i];
        track->onEndPlay(playInfo);
    }
}

void TimelinePlayer::stop()
{
    if (timeline == NULL)
        return;
    playing = false;
    for (int i = 0; i < timeline->tracks.size(); i++) {
        TimelineTrack* track = timeline->tracks[i];
        track->onEndPlay(playInfo);
    }
}

Serializable* TimelinePlayer::instantiate(const SerializationInfo& from)
{
    return new TimelinePlayer(from.name);
}

bool TimelinePlayer::deserialize(const SerializationInfo& from)
{
    if (!Object::deserialize(from))
        return false;

    const SerializationInfo* dataInfo = from.get("timeline");
    timeline.deserialize(*dataInfo);
    return true;
}

bool TimelinePlayer::serialize(SerializationInfo& to)
{
    if (!Object::serialize(to))
        return false;
    SerializationInfo* data = to.add("timeline");
    timeline.serialize(*data);
    return true;
}
