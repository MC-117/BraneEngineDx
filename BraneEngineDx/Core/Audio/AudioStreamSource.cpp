#include "AudioStreamSource.h"

SerializeInstance(AudioStreamSource);

bool AudioStreamSource::isValid() const
{
	return sbo != AL_NONE;
}

bool AudioStreamSource::setAudioData(AudioData* audioData)
{
	return stream(audioData);
}

bool AudioStreamSource::stream(AudioData* audioData)
{
	if (audioData == NULL || audioData->isLoad())
		return false;
	if (sbo == AL_NONE) {
		alGenSources(1, &sbo);
	}
	unsigned int abo = audioData->getBuffer();
	alSourceQueueBuffers(sbo, 1, &abo);
	return true;
}

void AudioStreamSource::clearQueue()
{
	int nbProcessed = 0;
	alGetSourcei(sbo, AL_BUFFERS_PROCESSED, &nbProcessed);
	alSourceUnqueueBuffers(sbo, nbProcessed, NULL);
}
