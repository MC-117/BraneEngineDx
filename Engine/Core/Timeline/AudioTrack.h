#pragma once

#include "TimelineClipTrack.h"
#include "AudioPlayable.h"

class AudioTrack : public TimelineClipTrack
{
public:
	Serialize(AudioTrack, TimelineClipTrack);

	AudioTrack(const string& name = "AudioTrack");
	virtual ~AudioTrack() = default;

	void setTransform(Transform * transform);
	Transform* getTransform() const;

	void setMaxVolume(float volume);
	float getMaxVolume() const;

	void setMute(bool mute);
	bool isMute() const;

	virtual void onPlayBlend(const PlayInfo & info);

	virtual void onUpdate(const PlayInfo & info);

	static Serializable* instantiate(const SerializationInfo & from);
	virtual bool deserialize(const SerializationInfo & from);
	virtual bool serialize(SerializationInfo & to);
protected:
	Ref<Transform> transform;
	float maxVolume = 1;
	bool mute = false;
};