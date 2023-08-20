#include "AudioStreamSource.h"
#include "../Console.h"

#undef min
#undef max

AudioStreamRingBuffer::AudioStreamRingBuffer(const Config& config) : config(config), frontIndex(-1)
{
	setConfig(config);
}

void AudioStreamRingBuffer::setConfig(const Config& config)
{
	this->config = config;
	audioBuffers.resize(config.bufferCount);
	for (auto& audioData : audioBuffers) {
		if (audioData == NULL)
			audioData = make_shared<AudioData>();
		audioData->wave.format = config;
		audioData->wave.data.resize(config.channels * config.framesPerBuffer * config.bitsPerSample / 8);
	}
}

void AudioStreamRingBuffer::resizeBuffer(int size)
{
	int index = audioBuffers.size();
	audioBuffers.resize(size);
	for (int i = index; i < size; i++) {
		auto& audioData = audioBuffers[i];
		audioData = make_shared<AudioData>();
		audioData->wave.format = config;
		audioData->wave.data.resize(config.channels * config.framesPerBuffer * config.bitsPerSample / 8);
	}
	frontIndex = -1;
}

const AudioData* AudioStreamRingBuffer::getFrontBuffer() const
{
	if (frontIndex < 0)
		return audioBuffers.empty() ? NULL : audioBuffers.front().get();
	return audioBuffers[frontIndex].get();
}

const AudioData* AudioStreamRingBuffer::getBackBuffer() const
{
	if (frontIndex < 0)
		return audioBuffers.empty() ? NULL : audioBuffers.back().get();
	return audioBuffers[(frontIndex + audioBuffers.size() - 1) % audioBuffers.size()].get();
}

AudioData* AudioStreamRingBuffer::fetchNextBuffer()
{
	if (frontIndex < 0)
		frontIndex = -1;
	frontIndex = (frontIndex + 1) % audioBuffers.size();
	return audioBuffers[frontIndex].get();
}

SerializeInstance(AudioStreamSource);

bool AudioStreamSource::isValid() const
{
	return sbo != AL_NONE;
}

bool AudioStreamSource::setAudioData(const AudioData* audioData)
{
	return stream(audioData);
}

bool AudioStreamSource::stream(const AudioData* audioData)
{
	if (audioData == NULL || !audioData->isLoad())
		return false;
	if (sbo == AL_NONE) {
		alGenSources(1, &sbo);
	}
	unsigned int abo;
	/*ALint processedCount = 0;
	alSourcei(sbo, AL_PROCESSED, &processedCount);
	if (processedCount > 0)
	alSourceUnqueueBuffers(sbo, 1, &abo);*/
	abo = audioData->getBuffer();
	alSourceQueueBuffers(sbo, 1, &abo);
	if (ALint err = alGetError())
		Console::error("Error Calling alSourceQueueBuffers: %d", err);
	return true;
}

void AudioStreamSource::clearQueue()
{
	if (sbo == AL_NONE)
		return;
	int nbProcessed = 0;
	alGetSourcei(sbo, AL_BUFFERS_PROCESSED, &nbProcessed);
	if (ALint err = alGetError())
		Console::error("Error Calling alGetSourcei: %d", err);
	if (nbProcessed > 0) {
		vector<unsigned int> values(nbProcessed);
		alSourceUnqueueBuffers(sbo, nbProcessed, values.data());
		if (ALint err = alGetError())
			Console::error("Error Calling alSourceUnqueueBuffers: %d", err);
	}
}

void AudioStreamSource::waitUntilUnprocessedRemain(unsigned int maxRemain)
{
	int queuedCount = 0;
	alGetSourcei(sbo, AL_BUFFERS_QUEUED, &queuedCount);
	if (queuedCount == 0)
		return;
	int nbProcessed = 0;
	do {
		this_thread::yield();
		alGetSourcei(sbo, AL_BUFFERS_PROCESSED, &nbProcessed);
	} while ((queuedCount - nbProcessed) > maxRemain);
	vector<unsigned int> values(nbProcessed);
	alSourceUnqueueBuffers(sbo, nbProcessed, values.data());
	if (ALint err = alGetError())
		Console::error("Error Calling alSourceUnqueueBuffers: %d", err);
}
