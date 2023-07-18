#pragma once
#ifndef _ACTOR_H_
#define _ACTOR_H_

#include "Transform.h"

#ifdef AUDIO_USE_OPENAL
#include "Audio/AudioSource.h"
#endif // AUDIO_USE_OPENAL

class Actor : public Transform
{
public:
	Serialize(Actor, Transform);

#ifdef AUDIO_USE_OPENAL
	vector<AudioSource*> audioSources;
#endif // AUDIO_USE_OPENAL

	Actor(string name = "Actor");
	virtual ~Actor();

	virtual void begin();
	virtual void tick(float deltaTime);
	virtual void afterTick();
	virtual void end();
	virtual void prerender(SceneRenderData& sceneData);

	virtual void setHidden(bool value);
	virtual bool isHidden();
#ifdef AUDIO_USE_OPENAL
	virtual AudioSource* addAudioSource(AudioData& audioData);
#endif // AUDIO_USE_OPENAL

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

#endif // !_ACTOR_H_
