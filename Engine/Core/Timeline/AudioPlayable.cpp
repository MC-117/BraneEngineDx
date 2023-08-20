#include "AudioPlayable.h"
#include "../Asset.h"

SerializeInstance(AudioPlayable);

void AudioPlayable::setAudio(AudioData* data)
{
    if (source.audioData != data) {
        source.setAudioData(data);
        targetTransform = NULL;
    }
}

AudioData* AudioPlayable::getAudio() const
{
    return source.audioData;
}

void AudioPlayable::setMaxVolume(float volume)
{
    maxVolume = volume;
}

float AudioPlayable::getMaxVolume() const
{
    return maxVolume;
}

void AudioPlayable::onBeginPlay(const PlayInfo& info)
{
    source.setPlayTime(info.currentTime);
    source.play();
}

void AudioPlayable::onPlay(const PlayInfo& info)
{
    if (targetTransform != NULL) {
        Vector3f position = targetTransform->getPosition(WORLD);
        source.setPosition(position);
        source.setDirection(targetTransform->getForward(WORLD));
        source.setVelocity((position - lastPosition) / info.deltaTime);
        lastPosition = position;
    }
}

void AudioPlayable::onEndPlay(const PlayInfo& info)
{
    source.pause();
}

Serializable* AudioPlayable::instantiate(const SerializationInfo& from)
{
    return new AudioPlayable();
}

bool AudioPlayable::deserialize(const SerializationInfo& from)
{
    if (!TimelinePlayable::deserialize(from))
        return false;
    string audio;
    if (from.get("audio", audio)) {
        AudioData* data = getAssetByPath<AudioData>(audio);
        if (data != NULL) {
            setAudio(data);
        }
    }
    return true;
}

bool AudioPlayable::serialize(SerializationInfo& to)
{
    if (!TimelinePlayable::serialize(to))
        return false;
    string audio;
    if (source.audioData != NULL) {
        audio = AssetInfo::getPath(source.audioData);
    }
    to.set("audio", audio);
    return true;
}

void AudioPlayable::bindTransform(Transform* transform)
{
    targetTransform = transform;
    if (targetTransform != NULL)
        lastPosition = targetTransform->getPosition(WORLD);
}
