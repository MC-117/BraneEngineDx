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

void Actor::prerender()
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
		return false; const SerializationInfo* ainfo = from.get("Audio");
#ifdef AUDIO_USE_OPENAL
	if (ainfo != NULL) {
		for (int i = 0; i < ainfo->stringList.size(); i++) {
			AudioData* audioData = getAssetByPath<AudioData>(ainfo->stringList[i]);
			if (audioData != NULL)
				addAudioSource(*audioData);
		}
	}
#endif // AUDIO_USE_OPENAL
	return true;
}

bool Actor::serialize(SerializationInfo & to)
{
	if (!::Transform::serialize(to))
		return false;
	to.type = "Actor";
#ifdef AUDIO_USE_OPENAL
	SerializationInfo* ainfo = to.add("Audio");
	if (ainfo == NULL)
		return false;
	ainfo->type = "Array";
	ainfo->arrayType = "String";
	for (int i = 0; i < audioSources.size(); i++) {
		string path = AudioDataAssetInfo::getPath(audioSources[i]->audioData);
		if (!path.empty())
			ainfo->push(path);
	}
#endif // AUDIO_USE_OPENAL
	return true;
}
