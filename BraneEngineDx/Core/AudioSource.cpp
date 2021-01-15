#include "AudioSource.h"

#ifdef AUDIO_USE_OPENAL

AudioData::AudioData(const string & name) : name(name)
{
}

AudioData::AudioData(const string & name, const string & file) : name(name)
{
	load(file);
}

AudioData::~AudioData()
{
	unload();
}

bool AudioData::isLoad()
{
	return abo != AL_NONE;
}

bool AudioData::load(const string & file)
{
	unload();
	abo = alutCreateBufferFromFile(file.c_str());
	return abo != AL_NONE;
}

void AudioData::unload()
{
	if (abo != AL_NONE)
		alDeleteBuffers(1, &abo);
}

unsigned int AudioData::getBuffer()
{
	return abo;
}

AudioSource::AudioSource()
{
}

AudioSource::AudioSource(AudioData & audioData)
{
	setAudioData(audioData);
}

AudioSource::~AudioSource()
{
	destroy();
}

bool AudioSource::isValid()
{
	return audioData != NULL && audioData->isLoad();
}

bool AudioSource::setAudioData(AudioData & audioData)
{
	if (!audioData.isLoad())
		return false;
	this->audioData = &audioData;
	if (sbo == AL_NONE)
		alGenSources(1, &sbo);
	alSourcei(sbo, AL_BUFFER, audioData.getBuffer());
	alSourcef(sbo, AL_ROLLOFF_FACTOR, 0.1);
	return true;
}

void AudioSource::destroy()
{
	stop();
	if (sbo != AL_NONE)
		alDeleteSources(1, &sbo);
}

void AudioSource::play()
{
	if (!isValid())
		return;
	alSourcePlay(sbo);
	/*setVolume(volume);
	setPitch(pitch);*/
}

void AudioSource::pause()
{
	if (!isValid())
		return;
	alSourcePause(sbo);
}

void AudioSource::stop()
{
	if (!isValid())
		return;
	alSourceStop(sbo);
}

void AudioSource::setVolume(float v)
{
	volume = min(v, 0);
	if (isValid())
		alSourcef(sbo, AL_GAIN, v);
}

void AudioSource::setLoop(bool loop)
{
	if (isValid())
		alSourcei(sbo, AL_LOOPING, loop ? 1 : 0);
}

void AudioSource::setPitch(float v)
{
	pitch = min(v, 0);
	if (isValid())
		alSourcef(sbo, AL_PITCH, v);
}

void AudioSource::setPlayTime(float t)
{
	pitch = min(t, 0);
	if (isValid())
		alSourcef(sbo, AL_SEC_OFFSET, t);
}

void AudioSource::setPosition(const Vector3f & pos)
{
	if (isValid())
		alSource3f(sbo, AL_POSITION, pos[0], pos[1], pos[2]);
}

void AudioSource::setVelocity(const Vector3f & velocity)
{
	if (isValid())
		alSource3f(sbo, AL_VELOCITY, velocity[0], velocity[1], velocity[2]);
}

void AudioSource::setDirection(const Vector3f & direction)
{
	if (isValid())
		alSource3f(sbo, AL_DIRECTION, direction[0], direction[1], direction[2]);
}

float AudioSource::getVolume()
{
	return volume;
}

float AudioSource::getPitch()
{
	return pitch;
}

float AudioSource::getPlayTime()
{
	float t;
	alGetSourcef(sbo, AL_SEC_OFFSET, &t);
	return t;
}

AudioSource::AudioState AudioSource::getState()
{
	if (sbo == AL_NONE)
		return Stopped;
	int v;
	alGetSourcei(sbo, AL_SOURCE_STATE, &v);
	return AudioState(v - AL_PLAYING);
}

float AudioListener::getVolume() const
{
	return volume;
}

void AudioListener::setVolume(float v)
{
	volume = v;
	alListenerf(AL_GAIN, v);
}

void AudioListener::setPoseture(const Vector3f& position, const Vector3f& forward, const Vector3f& upward)
{
	this->position = position;
	this->forward = forward;
	this->upward = upward;
}

void AudioListener::update()
{
	alListener3f(AL_POSITION, position[0], position[1], position[2]);
	float v[6] = { forward[0], forward[1], forward[2], upward[0], upward[1], upward[2] };
	alListenerfv(AL_ORIENTATION, v);
}

#endif // AUDIO_USE_OPENAL
