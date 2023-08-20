#include "AudioTrack.h"

SerializeInstance(AudioTrack);

AudioTrack::AudioTrack(const string& name) : TimelineClipTrack(name)
{
}

void AudioTrack::setTransform(Transform* transform)
{
    if (this->transform != transform) {
        this->transform = transform;
    }
}

Transform* AudioTrack::getTransform() const
{
    return transform;
}

void AudioTrack::setMaxVolume(float volume)
{
    maxVolume = volume;
}

float AudioTrack::getMaxVolume() const
{
    return maxVolume;
}

void AudioTrack::setMute(bool mute)
{
    this->mute = mute;
}

bool AudioTrack::isMute() const
{
    return mute;
}

void AudioTrack::onPlayBlend(const PlayInfo& info)
{
    for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++) {
        b->clip->onPlay(info);
    }
}

void AudioTrack::onUpdate(const PlayInfo& info)
{
    for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++) {
        AudioPlayable* playable = dynamic_cast<AudioPlayable*>(b->clip->playable);
        if (playable == NULL)
            continue;
        playable->bindTransform(transform);
        if (mute)
            playable->source.setVolume(0);
        else
            playable->source.setVolume(maxVolume * playable->maxVolume * b->weight);
    }
}

Serializable* AudioTrack::instantiate(const SerializationInfo& from)
{
    return new AudioTrack();
}

bool AudioTrack::deserialize(const SerializationInfo& from)
{
    if (!TimelineClipTrack::deserialize(from))
        return false;
    const SerializationInfo* transformInfo = from.get("transform");
    transform.deserialize(*transformInfo);
    return true;
}

bool AudioTrack::serialize(SerializationInfo& to)
{
    if (!TimelineClipTrack::serialize(to))
        return false;
    SerializationInfo* info = to.add("transform");
    transform.serialize(*info);
    return true;
}
