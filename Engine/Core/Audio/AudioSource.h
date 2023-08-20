#pragma once

#include "../Base.h"
#include "AudioWave.h"
#include "../InitializationManager.h"

#ifdef AUDIO_USE_OPENAL

#include <AL\al.h>
#include <AL\alc.h>
#include <AL\alext.h>

class AudioDevice;
class AudioDeviceManager : public Initialization
{
	friend class AudioDevice;
public:
	static AudioDeviceManager instance;

	set<AudioDevice*> devices;

	AudioDevice* getCurrentDevice();
protected:
	AudioDevice* currentDevice;
	AudioDeviceManager();
	virtual bool initialize();
	virtual bool finalize();
};

class AudioDevice
{
public:
	AudioDevice();
	virtual ~AudioDevice();

	bool create(const string& name);
	bool release();

	bool setCurrentDevice();
protected:
	string name;
	ALCdevice* device;
	ALCcontext* context;
};

class AudioData
{
public:
	string name;
	AudioWave wave;

	AudioData(const string& name = "");
	AudioData(const string& name, const string& file);
	~AudioData();

	bool isLoad() const;

	unsigned int getRawBufferSize() const;
	const char* getRawBuffer() const;
	unsigned int getFrequency() const;
	unsigned int getChannels() const;
	unsigned int getBitsPerSample() const;
	float getDuration() const;
	bool getLoopPoint(Vector2i& point);

	bool load(const string& file);
	void reload();
	void unload();

	unsigned int getBuffer() const;
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

	virtual bool isValid() const;
	virtual bool setAudioData(AudioData* audioData);
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

#endif // AUDIO_USE_OPENAL
