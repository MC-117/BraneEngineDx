#pragma once

#include "TimelineTrack.h"
#include "AnimationPlayable.h"

class AnimationTrack : public TimelineTrack
{
public:
	Serialize(AnimationTrack, TimelineTrack);

	AnimationTrack(const string& name = "AnimationTrack");
	virtual ~AnimationTrack() = default;

	void setActor(SkeletonMeshActor * actor);
	SkeletonMeshActor* getActor() const;

	virtual void onBeginPlay(const PlayInfo& info);

	virtual void onPlayBlend(const PlayInfo& info);

	virtual void onUpdate(const PlayInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	Ref<SkeletonMeshActor> actor;
	AnimationPose pose;

	bool initActor = false;

	void bindActor();
};