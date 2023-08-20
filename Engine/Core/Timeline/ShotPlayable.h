#pragma once

#include "TimelinePlayable.h"
#include "../Camera.h"

class ShotPlayable : public TimelinePlayable
{
public:
	friend class ShotTrack;
public:
	Serialize(ShotPlayable, TimelinePlayable);

	ShotPlayable() = default;
	virtual ~ShotPlayable() = default;

	void setCamera(Camera* camera);
	Camera* getCamera() const;

	void setAnimation(AnimationClipData * data);
	AnimationClipData* getAnimation() const;

	virtual void onBeginPlay(const PlayInfo & info);
	virtual void onPlay(const PlayInfo & info);
	virtual void onEndPlay(const PlayInfo & info);

	void updateCamera(float deltaTime);

	static Serializable* instantiate(const SerializationInfo & from);
	virtual bool deserialize(const SerializationInfo & from);
	virtual bool serialize(SerializationInfo & to);
protected:
	AnimationClip clip;
	Ref<Camera> targetCamera;
};