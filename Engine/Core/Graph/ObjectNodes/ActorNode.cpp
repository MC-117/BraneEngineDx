#include "ActorNode.h"
#include "../../Actor.h"

IMP_REF_PIN(AudioSource, AudioSourceRefPin, Color(218, 0, 183));
IMP_REF_VAR_CLASS(AudioSource, AudioSourceRefVariable, AudioSourceRefPin, Color(218, 0, 183));

IMP_REF_PIN(Actor, ActorRefPin, Color(218, 0, 183));
IMP_REF_VAR_CLASS(Actor, ActorRefVariable, ActorRefPin, Color(218, 0, 183));

SerializeInstance(AudioSourcePlayNode);

AudioSourcePlayNode::AudioSourcePlayNode()
{
    displayName = "Play";
    audioPin = new AudioSourceRefPin("AudioSource");
    addInput(*audioPin);
}

bool AudioSourcePlayNode::process(GraphContext& context)
{
    AudioSource* audio = audioPin->getRef();
    if (audio)
        audio->play();
    return true;
}

Serializable* AudioSourcePlayNode::instantiate(const SerializationInfo& from)
{
    return new AudioSourcePlayNode();
}

SerializeInstance(AudioSourcePauseNode);

AudioSourcePauseNode::AudioSourcePauseNode()
{
    displayName = "Pause";
    audioPin = new AudioSourceRefPin("AudioSource");
    addInput(*audioPin);
}

bool AudioSourcePauseNode::process(GraphContext& context)
{
    AudioSource* audio = audioPin->getRef();
    if (audio)
        audio->pause();
    return true;
}

Serializable* AudioSourcePauseNode::instantiate(const SerializationInfo& from)
{
    return new AudioSourcePauseNode();
}

SerializeInstance(AudioSourceStopNode);

AudioSourceStopNode::AudioSourceStopNode()
{
    displayName = "Stop";
    audioPin = new AudioSourceRefPin("AudioSource");
    addInput(*audioPin);
}

bool AudioSourceStopNode::process(GraphContext& context)
{
    AudioSource* audio = audioPin->getRef();
    if (audio)
        audio->stop();
    return true;
}

Serializable* AudioSourceStopNode::instantiate(const SerializationInfo& from)
{
    return new AudioSourceStopNode();
}

SerializeInstance(AudioSourceIsPlayingNode);

AudioSourceIsPlayingNode::AudioSourceIsPlayingNode()
{
    displayName = "IsPlaying";
    audioPin = new AudioSourceRefPin("AudioSource");
    returnPin = new BoolPin("Return");
    addInput(*audioPin);
    addOutput(*returnPin);
}

bool AudioSourceIsPlayingNode::process(GraphContext& context)
{
    AudioSource* audio = audioPin->getRef();
    bool isPlaying = false;
    if (audio)
        isPlaying = audio->getState() == AudioSource::Playing;
    returnPin->setValue(isPlaying);
    return true;
}

Serializable* AudioSourceIsPlayingNode::instantiate(const SerializationInfo& from)
{
    return new AudioSourceIsPlayingNode();
}

SerializeInstance(AudioSourceGetTimeNode);

AudioSourceGetTimeNode::AudioSourceGetTimeNode()
{
    displayName = "GetTime";
    audioPin = new AudioSourceRefPin("AudioSource");
    timePin = new FloatPin("Time");
    addInput(*audioPin);
    addOutput(*timePin);
}

bool AudioSourceGetTimeNode::process(GraphContext& context)
{
    AudioSource* audio = audioPin->getRef();
    float time = 0;
    if (audio)
        time = audio->getPlayTime();
    timePin->setValue(time);
    return true;
}

Serializable* AudioSourceGetTimeNode::instantiate(const SerializationInfo& from)
{
    return new AudioSourceGetTimeNode();
}

SerializeInstance(AudioSourceSetTimeNode);

AudioSourceSetTimeNode::AudioSourceSetTimeNode()
{
    displayName = "SetTime";
    audioPin = new AudioSourceRefPin("AudioSource");
    timePin = new FloatPin("Time");
    addInput(*audioPin);
    addInput(*timePin);
}

bool AudioSourceSetTimeNode::process(GraphContext& context)
{
    AudioSource* audio = audioPin->getRef();
    float time = timePin->getValue();
    if (audio)
        audio->setPlayTime(time);
    return true;
}

Serializable* AudioSourceSetTimeNode::instantiate(const SerializationInfo& from)
{
    return new AudioSourceSetTimeNode();
}

SerializeInstance(AudioSourceGetVolumeNode);

AudioSourceGetVolumeNode::AudioSourceGetVolumeNode()
{
    displayName = "GetVolime";
    audioPin = new AudioSourceRefPin("AudioSource");
    volumePin = new FloatPin("Volume");
    addInput(*audioPin);
    addOutput(*volumePin);
}

bool AudioSourceGetVolumeNode::process(GraphContext& context)
{
    AudioSource* audio = audioPin->getRef();
    float volume = 0;
    if (audio)
        volume = audio->getVolume();
    volumePin->setValue(volume);
    return true;
}

Serializable* AudioSourceGetVolumeNode::instantiate(const SerializationInfo& from)
{
    return new AudioSourceGetVolumeNode();
}

SerializeInstance(AudioSourceSetVolumeNode);

AudioSourceSetVolumeNode::AudioSourceSetVolumeNode()
{
    displayName = "SetVolime";
    audioPin = new AudioSourceRefPin("AudioSource");
    volumePin = new FloatPin("Volume");
    addInput(*audioPin);
    addInput(*volumePin);
}

bool AudioSourceSetVolumeNode::process(GraphContext& context)
{
    AudioSource* audio = audioPin->getRef();
    float volume = volumePin->getValue();
    if (audio)
        audio->setVolume(volume);
    return true;
}

Serializable* AudioSourceSetVolumeNode::instantiate(const SerializationInfo& from)
{
    return new AudioSourceSetVolumeNode();
}

SerializeInstance(ActorGetAudioSourceCountNode);

ActorGetAudioSourceCountNode::ActorGetAudioSourceCountNode()
{
    displayName = "GetAudioSourceCount";
    actorPin = new ActorRefPin("Actor");
    countPin = new IntPin("Count");
    addInput(*actorPin);
    addOutput(*countPin);
}

bool ActorGetAudioSourceCountNode::process(GraphContext& context)
{
    Actor* actor = actorPin->getRef();
    int count = 0;
    if (actor)
        count = actor->audioSources.size();
    countPin->setValue(count);
    return true;
}

Serializable* ActorGetAudioSourceCountNode::instantiate(const SerializationInfo& from)
{
    return new ActorGetAudioSourceCountNode();
}

SerializeInstance(ActorGetAudioSourceNode);

ActorGetAudioSourceNode::ActorGetAudioSourceNode()
{
    displayName = "GetAudioSource";
    actorPin = new ActorRefPin("Actor");
    indexPin = new IntPin("Index");
    audioPin = new AudioSourceRefPin("AudioSource");
    addInput(*actorPin);
    addInput(*indexPin);
    addOutput(*audioPin);
}

bool ActorGetAudioSourceNode::process(GraphContext& context)
{
    Actor* actor = actorPin->getRef();
    float index = indexPin->getValue();
    AudioSource* audio = NULL;
    if (actor && index < actor->audioSources.size())
        audio = actor->audioSources[index];
    audioPin->setRef(audio);
    return true;
}

Serializable* ActorGetAudioSourceNode::instantiate(const SerializationInfo& from)
{
    return new ActorGetAudioSourceNode();
}
