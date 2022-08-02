#pragma once

#include "AnimationState.h"
#include "../Graph/StateMachine/StateMachine.h"

class AnimationStateMachine : public StateMachine, public IAnimationPoseNode
{
public:
	Serialize(AnimationStateMachine, StateMachine);

	AnimationStateMachine();
	virtual ~AnimationStateMachine() = default;

	virtual void setContext(const AnimationContext& context);
	virtual AnimationPose* getPose();

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	SkeletonMeshActorRefPin* actorPin;
	AnimationPosePin* posePin;
};