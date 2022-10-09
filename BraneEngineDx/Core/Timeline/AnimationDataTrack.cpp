#include "AnimationDataTrack.h"

SerializeInstance(AnimationDataTrack);

AnimationDataTrack::AnimationDataTrack(const string& name) : TimelineTrack(name)
{
}

void AnimationDataTrack::apply()
{
	AnimationClipData* data = getAnimationData();
	if (data) {
		startTime = 0;
		duration = data->duration;
	}
	else {
		startTime = 0;
		duration = 0;
	}
}

void AnimationDataTrack::setActor(SkeletonMeshActor* actor)
{
	this->actor = actor;
	bindActor();
	apply();
}

SkeletonMeshActor* AnimationDataTrack::getActor() const
{
	return actor;
}

void AnimationDataTrack::setAnimationData(AnimationClipData* data)
{
	playable.setAnimation(data);
	apply();
}

AnimationClipData* AnimationDataTrack::getAnimationData() const
{
	return playable.getAnimation();
}

void AnimationDataTrack::onBeginPlay(const PlayInfo& info)
{
	bindActor();
	playable.onBeginPlay(info);
	onPlay(info);
	TimelineTrack::onBeginPlay(info);
}

void AnimationDataTrack::onPlay(const PlayInfo& info)
{
	playable.onPlay(info);
	onUpdate(info);
}

void AnimationDataTrack::onEndPlay(const PlayInfo& info)
{
	playable.onEndPlay(info);
}

void AnimationDataTrack::onUpdate(const PlayInfo& info)
{
	SkeletonMeshActor* pActor = actor;
	Enum<AnimationUpdateFlags> updateFlags = playable.clip.getUpdateFlags();
	playable.clip.getPose().applyPose(updateFlags);
}

Serializable* AnimationDataTrack::instantiate(const SerializationInfo& from)
{
	return new AnimationDataTrack();
}

bool AnimationDataTrack::deserialize(const SerializationInfo& from)
{
	if (!TimelineTrack::deserialize(from))
		return false;
	const SerializationInfo* actorInfo = from.get("actor");
	actor.deserialize(*actorInfo);
	const SerializationInfo* playableInfo = from.get("playable");
	playable.deserialize(*playableInfo);
	return true;
}

bool AnimationDataTrack::serialize(SerializationInfo& to)
{
	if (!TimelineTrack::serialize(to))
		return false;
	SerializationInfo* actorInfo = to.add("actor");
	actor.serialize(*actorInfo);
	SerializationInfo* playableInfo = to.add("playable");
	playable.serialize(*playableInfo);
	return true;
}

void AnimationDataTrack::bindActor()
{
	playable.bindActor(actor);
}
