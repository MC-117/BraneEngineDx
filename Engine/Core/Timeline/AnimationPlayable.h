#pragma once

#include "TimelinePlayable.h"
#include "../SkeletonMeshActor.h"

class AnimationTrack;

class AnimationPlayable : public TimelinePlayable
{
	friend class AnimationTrack;
public:
	Serialize(AnimationPlayable, TimelinePlayable);

	AnimationClip clip;

	AnimationPlayable() = default;
	virtual ~AnimationPlayable() = default;

	void setAnimation(AnimationClipData* data);
	AnimationClipData* getAnimation() const;

	void setAnimationBlendMode(AnimationBlendMode mode);
	AnimationBlendMode getAnimationBlendMode() const;

	void bindActor(SkeletonMeshActor* actor);

	virtual void onBeginPlay(const PlayInfo& info);
	virtual void onPlay(const PlayInfo& info);
	virtual void onEndPlay(const PlayInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	AnimationBlendMode blendMode = AnimationBlendMode::Replace;
	bool isPlaying = false;

	SkeletonMeshActor* targetActor = NULL;
};