#include "AudioSource.h"
#include "../Asset.h"
#include <fstream>

#ifdef AUDIO_USE_OPENAL

AudioDeviceManager AudioDeviceManager::instance;

AudioDevice* AudioDeviceManager::getCurrentDevice()
{
	return currentDevice;
}

AudioDeviceManager::AudioDeviceManager() : Initialization(
	InitializeStage::BeforeAssetLoading, 0,
	FinalizeStage::BeforeRenderVenderRelease, 0)
{
}

bool AudioDeviceManager::initialize()
{
	AudioDevice* defaultDevice = new AudioDevice();
	defaultDevice->create("");
	return defaultDevice->setCurrentDevice();
}

bool AudioDeviceManager::finalize()
{
	set<AudioDevice*> tempDevices = devices;
	for (auto device : tempDevices)
		delete device;
}

AudioDevice::AudioDevice()
{
	AudioDeviceManager::instance.devices.insert(this);
}

AudioDevice::~AudioDevice()
{
	release();
	AudioDeviceManager::instance.devices.erase(this);
}

bool AudioDevice::create(const string& name)
{
	device = alcOpenDevice(name.c_str());
	context = alcCreateContext(device, nullptr);
	return context;
}

bool AudioDevice::release()
{
	if (AudioDeviceManager::instance.currentDevice == this) {
		alcMakeContextCurrent(NULL);
		AudioDeviceManager::instance.currentDevice = NULL;
	}
	alcDestroyContext(context);
	bool success = alcCloseDevice(device);
	context = NULL;
	device = NULL;
	return success;
}

bool AudioDevice::setCurrentDevice()
{
	if (context && alcMakeContextCurrent(context)) {
		AudioDeviceManager::instance.currentDevice = this;
		return true;
	}
	return false;
}

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

unsigned int AudioData::getRawBufferSize() const
{
	return wave.data.size();
}

const char* AudioData::getRawBuffer() const
{
	return wave.data.data();
}

unsigned int AudioData::getFrequency() const
{
	return wave.format.sampleRate;
}

unsigned int AudioData::getChannels() const
{
	return wave.format.channels;
}

unsigned int AudioData::getBitsPerSample() const
{
	return wave.format.bitsPerSample;
}

float AudioData::getDuration() const
{
	return ((float)getRawBufferSize()) / (getFrequency() * getChannels() * (getBitsPerSample() / 8));
}

bool AudioData::getLoopPoint(Vector2i& point)
{
	bool hasLoop = !wave.loops.empty();
	if (hasLoop) {
		AudioWave::SampleLoop& loop = wave.loops.front();
		point = { loop.start, loop.end };
	}
	return hasLoop;
}

bool AudioData::load(const string & file)
{
	ifstream is(file, ios::binary);
	if (is.fail())
		return false;

	unload();

	wave.loadFromStream(is);

	alGenBuffers(1, &abo);
	if (wave.format.channels == 1) {
		if (wave.format.bitsPerSample == 8) {
			alBufferData(abo, AL_FORMAT_MONO8, wave.data.data(), wave.data.size(), wave.format.sampleRate);
		}
		else {
			alBufferData(abo, AL_FORMAT_MONO16, wave.data.data(), wave.data.size(), wave.format.sampleRate);
		}
	}
	else {
		if (wave.format.bitsPerSample == 8) {
			alBufferData(abo, AL_FORMAT_STEREO8, wave.data.data(), wave.data.size(), wave.format.sampleRate);
		}
		else {
			alBufferData(abo, AL_FORMAT_STEREO16, wave.data.data(), wave.data.size(), wave.format.sampleRate);
		}
	}

	return abo != AL_NONE;
}

void AudioData::unload()
{
	if (abo != AL_NONE)
		alDeleteBuffers(1, &abo);
	wave.reset();
}

unsigned int AudioData::getBuffer()
{
	return abo;
}

SerializeInstance(AudioSource);

AudioSource::AudioSource() : Base()
{
}

AudioSource::AudioSource(AudioData & audioData) : Base()
{
	setAudioData(&audioData);
}

AudioSource::~AudioSource()
{
	destroy();
}

bool AudioSource::isValid()
{
	return audioData != NULL && audioData->isLoad();
}

bool AudioSource::setAudioData(AudioData* audioData)
{
	if (audioData == NULL) {
		destroy();
		this->audioData = NULL;
		return true;
	}
	if (!audioData->isLoad())
		return false;
	this->audioData = audioData;
	if (sbo == AL_NONE)
		alGenSources(1, &sbo);
	alSourcei(sbo, AL_BUFFER, audioData->getBuffer());
	alSourcef(sbo, AL_ROLLOFF_FACTOR, 0.1);

	Vector2i loopPoint;
	if (audioData->getLoopPoint(loopPoint)) {
		alSourcei(sbo, AL_LOOPING, 1);
		alSourceiv(sbo, AL_LOOP_POINTS_SOFT, loopPoint.data());
	}
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
	pitch = max(v, 0);
	if (isValid())
		alSourcef(sbo, AL_PITCH, v);
}

void AudioSource::setPlayTime(float t)
{
	t = max(t, 0);
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

Serializable* AudioSource::instantiate(const SerializationInfo& from)
{
	return new AudioSource();
}

bool AudioSource::deserialize(const SerializationInfo& from)
{
	Base::deserialize(from);
	string path;
	if (from.get("audioData", path)) {
		AudioData* data = getAssetByPath<AudioData>(path);
		if (data) {
			setAudioData(data);
		}
	}
	return true;
}

bool AudioSource::serialize(SerializationInfo& to)
{
	Base::serialize(to);
	string path = AssetInfo::getPath(audioData);
	to.set("audioData", path);
	return true;
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
