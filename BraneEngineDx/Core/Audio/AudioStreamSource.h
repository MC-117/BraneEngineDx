#pragma once

#include "AudioSource.h"

class AudioStreamSource : public AudioSource
{
public:
	Serialize(AudioStreamSource, AudioSource);

	AudioStreamSource() = default;

	virtual bool isValid() const;
	virtual bool setAudioData(AudioData* audioData);

	virtual bool stream(AudioData* audioData);
	virtual void clearQueue();
};