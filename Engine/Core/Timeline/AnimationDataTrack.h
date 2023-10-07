#pragma once

#include "TimelineTrack.h"
#include "AnimationPlayable.h"

class ENGINE_API AnimationDataTrack : public TimelineTrack
{
public:
	Serialize(AnimationDataTrack, TimelineTrack);

	AnimationPlayable playable;

	AnimationDataTrack(const string& name = "AnimationDataTrack");
	virtual ~AnimationDataTrack() = default;

	virtual void apply();

	void setActor(SkeletonMeshActor* actor);
	SkeletonMeshActor* getActor() const;

	void setAnimationData(AnimationClipData* data);
	AnimationClipData* getAnimationData() const;

	virtual void onBeginPlay(const PlayInfo& info);
	virtual void onPlay(const PlayInfo& info);
	virtual void onEndPlay(const PlayInfo& info);

	virtual void onUpdate(const PlayInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	Ref<SkeletonMeshActor> actor;

	void bindActor();
};