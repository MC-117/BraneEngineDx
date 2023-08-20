#include "Actor.h"
#include "Asset.h"

SerializeInstance(Actor);

Actor::Actor(string name) : Transform(name)
{
}

Actor::~Actor()
{
}

void Actor::begin()
{
	Transform::begin();
}

void Actor::tick(float deltaTime)
{
	Transform::tick(deltaTime);
}

void Actor::afterTick()
{
	Transform::afterTick();
}

void Actor::end()
{
	::Transform::end();
}

void Actor::prerender(SceneRenderData& sceneData)
{
}

void Actor::setHidden(bool value)
{
}

bool Actor::isHidden()
{
	return false;
}

#ifdef AUDIO_USE_OPENAL
AudioSource* Actor::addAudioSource(AudioData& audioData)
{
	if (!audioData.isLoad())
		return NULL;
	AudioSource* s = new AudioSource(audioData);
	audioSources.push_back(s);
	return s;
}
#endif // AUDIO_USE_OPENAL

Serializable * Actor::instantiate(const SerializationInfo & from)
{
	return new Actor(from.name);
}

bool Actor::deserialize(const SerializationInfo & from)
{
	if (!::Transform::deserialize(from))
		return false;
#ifdef AUDIO_USE_OPENAL
	const SerializationInfo* ainfo = from.get("Audio");
	if (ainfo != NULL) {
		for (int i = 0; i < ainfo->sublists.size(); i++) {
			const SerializationInfo& asInfo = ainfo->sublists[i];
			AudioSource* source = new AudioSource();
			source->deserialize(asInfo);
			audioSources.push_back(source);
		}
	}
#endif // AUDIO_USE_OPENAL
	return true;
}

bool Actor::serialize(SerializationInfo & to)
{
	if (!::Transform::serialize(to))
		return false;
#ifdef AUDIO_USE_OPENAL
	SerializationInfo* ainfo = to.add("Audio");
	if (ainfo != NULL)
	{
		ainfo->type = "AudioSources";
		for (int i = 0; i < audioSources.size(); i++) {
			SerializationInfo* asInfo = ainfo->add(to_string(i));
			audioSources[i]->serialize(*asInfo);
		}
	}
#endif // AUDIO_USE_OPENAL
	return true;
}
