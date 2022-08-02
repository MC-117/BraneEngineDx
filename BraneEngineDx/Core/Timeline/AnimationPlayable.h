#pragma once

#include "TimelinePlayable.h"
#include "../SkeletonMeshActor.h"

class AnimationTrack;

class AnimationPlayable : public TimelinePlayable
{
	friend class AnimationTrack;
public:
	Serialize(AnimationPlayable, TimelinePlayable);

	AnimationPlayable() = default;
	virtual ~AnimationPlayable() = default;

	void setAnimation(AnimationClipData* data);
	AnimationClipData* getAnimation() const;

	void setAnimationBlendMode(AnimationBlendMode mode);
	AnimationBlendMode getAnimationBlendMode() const;

	virtual void onBeginPlay(const PlayInfo& info);
	virtual void onPlay(const PlayInfo& info);
	virtual void onEndPlay(const PlayInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	AnimationClip clip;
	AnimationBlendMode blendMode = AnimationBlendMode::Replace;

	SkeletonMeshActor* targetActor = NULL;
	AnimationPose bindPose;
	void bindActor(SkeletonMeshActor* actor);
};