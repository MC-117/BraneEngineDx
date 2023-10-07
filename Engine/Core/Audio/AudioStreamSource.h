#pragma once

#include "AudioSource.h"

class ENGINE_API AudioStreamRingBuffer
{
public:
	struct Config : AudioWave::FormatChunk
	{
		int framesPerBuffer;
		int bufferCount;
	};

	AudioStreamRingBuffer() = default;
	AudioStreamRingBuffer(const Config& config);

	void setConfig(const Config& config);
	void resizeBuffer(int size);
	const AudioData* getFrontBuffer() const;
	const AudioData* getBackBuffer() const;
	AudioData* fetchNextBuffer();
protected:
	Config config;
	vector<shared_ptr<AudioData>> audioBuffers;
	int frontIndex;
};

class ENGINE_API AudioStreamSource : public AudioSource
{
public:
	Serialize(AudioStreamSource, AudioSource);

	AudioStreamSource() = default;

	virtual bool isValid() const;
	virtual bool setAudioData(const AudioData* audioData);

	virtual bool stream(const AudioData* audioData);
	virtual void clearQueue();

	virtual void waitUntilUnprocessedRemain(unsigned int maxRemain);
};