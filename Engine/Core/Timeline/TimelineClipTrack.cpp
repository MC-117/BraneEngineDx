#include "TimelineClipTrack.h"

SerializeInstance(TimelineClipTrack);

TimelineClipTrack::TimelineClipTrack(const string& name) : TimelineTrack(name)
{
}

TimelineClipTrack::~TimelineClipTrack()
{
	for (int i = 0; i < clips.size(); i++) {
		TimelineClip* clip = clips[i];
		delete clip;
	}
	clips.clear();
	activedClips.clear();
}

TimelineClip* TimelineClipTrack::addClip(const ClipInfo& info)
{
    TimelineClip* clip = new TimelineClip();
    clip->startTime = info.startTime;
    clip->duration = info.duration;
    clip->playable = info.playable;
    clip->blendInDuration = 0;
    clip->blendOutDuration = 0;
    if (info.playable != NULL) {
        clip->name = info.playable->getSerialization().type;
        const char playableStr[] = "Playable";
        size_t pos = clip->name.find(playableStr);
        if (pos != string::npos) {
            clip->name.erase(pos, sizeof(playableStr) - 1);
        }
        clip->name += "Clip";
    }
    clips.push_back(clip);
    apply();
    return clip;
}

bool TimelineClipTrack::removeClip(int index)
{
    if (index < 0 || index >= clips.size())
        return false;
    clips.erase(clips.begin() + index);
    apply();
    return true;
}

void TimelineClipTrack::apply()
{
    sort(clips.begin(), clips.end(),
        [](TimelineClip* a, TimelineClip* b)
        {
            if (a->startTime < b->startTime)
                return true;
            else if (a->startTime == b->startTime)
                return a->duration < b->duration;
            return false;
        });
    TimelineClip* lastClip = NULL;
    for (int i = 0; i < clips.size(); i++) {
        TimelineClip* clip = clips[i];
        clip->index = i;
        if (lastClip != NULL) {
            float blend = lastClip->startTime +
                lastClip->duration - clip->startTime;
            if (blend > 0 && blend < clip->duration) {
                lastClip->blendOutDuration = blend;
                clip->blendInDuration = blend;
            }
            else {
                lastClip->blendOutDuration = 0;
                clip->blendInDuration = 0;
            }
        }
        lastClip = clip;
    }
    if (clips.empty()) {
        startTime = duration = 0;
    }
    else {
        startTime = clips.front()->startTime;
        TimelineClip* backClip = clips.back();
        duration = backClip->startTime + backClip->duration - startTime;
    }
}

void TimelineClipTrack::onBeginPlay(const PlayInfo& info)
{
    activedClips.clear();
    bool found = false;
    for (int i = 0; i < clips.size(); i++) {
        TimelineClip* clip = clips[i];
        float weight = clip->getBlendWeight(info);
        if (weight == 0) {
            if (found)
                break;
        }
        else {
            found = true;
        }
        activedClips.push_back({ clip, weight });
        clip->onBeginPlay(info);
    }
    int blendSize = activedClips.size();
    if (blendSize == 1)
        activedClips.front().clip->onPlay(info);
    else if (blendSize > 1) {
        calculateActiveClipWeight();
        onPlayBlend(info);
    }
    onUpdate(info);
}

void TimelineClipTrack::onPlay(const PlayInfo& info)
{
    int startIndex = 0;
    if (!activedClips.empty()) {
        startIndex = activedClips.front().clip->index;
    }
    int blendSize = activedClips.size();
    int lastMaxIndex = startIndex + blendSize;
    bool blend = blendSize > 1;
    activedClips.clear();
    for (int i = startIndex; i < clips.size(); i++) {
        TimelineClip* clip = clips[i];
        bool lastInClip = i < lastMaxIndex;
        bool inClip = clip->isInClip(info);
        if (!activedClips.empty() && !inClip)
            break;
        if (lastInClip && !inClip)
            clip->onEndPlay(info);
        else if (!lastInClip && inClip)
            clip->onBeginPlay(info);
        activedClips.push_back({ clip, clip->getBlendWeight(info) });
    }
    blendSize = activedClips.size();
    if (blendSize == 1)
        activedClips.front().clip->onPlay(info);
    else if (blendSize > 1) {
        calculateActiveClipWeight();
        onPlayBlend(info);
    }
    onUpdate(info);
}

void TimelineClipTrack::onEndPlay(const PlayInfo& info)
{
    int blendSize = activedClips.size();
    if (blendSize == 0)
        return;
    else if (blendSize == 1)
        activedClips.front().clip->onPlay(info);
    else if (blendSize > 1) {
        calculateActiveClipWeight();
        onPlayBlend(info);
    }
    for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++) {
        (*b).clip->onEndPlay(info);
    }
    activedClips.clear();
}

void TimelineClipTrack::onPlayBlend(const PlayInfo& info)
{
    activedClips.back().clip->onPlay(info);
}

void TimelineClipTrack::onUpdate(const PlayInfo& info)
{
}

Serializable* TimelineClipTrack::instantiate(const SerializationInfo& from)
{
    return new TimelineClipTrack();
}

bool TimelineClipTrack::deserialize(const SerializationInfo& from)
{
    TimelineTrack::deserialize(from);
    const SerializationInfo* clipInfos = from.get("clips");
    if (clipInfos != NULL)
        for (auto b = clipInfos->sublists.begin(), e = clipInfos->sublists.end(); b != e; b++) {
            if (b->serialization == &TimelineClip::TimelineClipSerialization::serialization) {
                TimelineClip* clip = new TimelineClip();
                clip->deserialize(*b);
                clips.push_back(clip);
            }
        }
    apply();
    return true;
}

bool TimelineClipTrack::serialize(SerializationInfo& to)
{
    TimelineTrack::serialize(to);
    SerializationInfo* info = to.addArray<TimelineClip>("clips");
    for (int i = 0; i < clips.size(); i++) {
        TimelineClip* clip = clips[i];
        SerializationInfo* clipInfo = info->push();
        clip->serialize(*clipInfo);
    }
    return true;
}

void TimelineClipTrack::calculateActiveClipWeight()
{
    float sum = 0;
    int i = 0;
    for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++, i++) {
        sum += b->weight;
    }
    i = 0;
    for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++, i++) {
        b->weight /= sum;
    }
}
