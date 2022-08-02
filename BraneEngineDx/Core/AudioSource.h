#pragma once

#include "Base.h"

#ifdef AUDIO_USE_OPENAL

#ifndef _AUDIOSOURCE_H_
#define _AUDIOSOURCE_H_

#include <AL\alut.h>

class AudioData
{
public:
	string name;
	unsigned int bufferSize = 0;
	int frequency;
	int channels = 0;
	int bitsPerSample = 0;
	float duration = 0;

	AudioData(const string& name = "");
	AudioData(const string& name, const string& file);
	~AudioData();

	bool isLoad();

	bool load(const string& file);
	void unload();

	unsigned int getBuffer();
protected:
	unsigned int abo = AL_NONE;
};

class AudioSource : public Base
{
public:
	enum AudioState
	{
		Playing, Paused, Stopped
	};
	AudioData* audioData = NULL;
	float volume = 1;
	float pitch = 1;

	Serialize(AudioSource, Base);

	AudioSource();
	AudioSource(AudioData& audioData);
	virtual ~AudioSource();

	bool isValid();
	bool setAudioData(AudioData* audioData);
	void destroy();

	void play();
	void pause();
	void stop();

	void setVolume(float v);
	void setLoop(bool loop);
	void setPitch(float v);
	void setPlayTime(float t);
	void setPosition(const Vector3f& pos);
	void setVelocity(const Vector3f& velocity);
	void setDirection(const Vector3f& direction);

	float getVolume();
	float getPitch();
	float getPlayTime();
	AudioState getState();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	unsigned int sbo = AL_NONE;
};

class AudioListener
{
public:
	Vector3f position;
	Vector3f forward;
	Vector3f upward;
	float volume = 1;

	virtual float getVolume() const;
	virtual void setVolume(float v);
	virtual void setPoseture(const Vector3f& position, const Vector3f& forward, const Vector3f& upward);
	virtual void update();
};

#endif // !_AUDIOSOURCE_H_

#endif // AUDIO_USE_OPENAL
