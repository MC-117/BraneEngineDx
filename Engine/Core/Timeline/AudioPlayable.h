#pragma once

#include "TimelinePlayable.h"
#include "../Audio/AudioSource.h"
#include "../Transform.h"

class ENGINE_API AudioPlayable : public TimelinePlayable
{
	friend class AudioTrack;
public:
	Serialize(AudioPlayable, TimelinePlayable);

	AudioPlayable() = default;
	virtual ~AudioPlayable() = default;

	void setAudio(AudioData* data);
	AudioData* getAudio() const;

	void setMaxVolume(float volume);
	float getMaxVolume() const;

	virtual void onBeginPlay(const PlayInfo& info);
	virtual void onPlay(const PlayInfo& info);
	virtual void onEndPlay(const PlayInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	AudioSource source;
	float maxVolume = 1;
	Vector3f lastPosition;
	Transform* targetTransform = NULL;
	void bindTransform(Transform* transform);
};