#include "AnimationStateMachine.h"
#include "../SkeletonMeshActor.h"

SerializeInstance(AnimationStateMachine);

AnimationStateMachine::AnimationStateMachine() : StateMachine()
{
	name = "AnimationStateMachine";
	actorPin = new SkeletonMeshActorRefPin("Target");
	posePin = new AnimationPosePin("Pose");
	addInput(*actorPin);
	addOutput(*posePin);
}

void AnimationStateMachine::setContext(const AnimationContext& context)
{
}

AnimationPose* AnimationStateMachine::getPose()
{
	return posePin->getPose();
}

bool AnimationStateMachine::process(GraphContext& context)
{
	AnimationGraphContext localContext;
	localContext.deltaTime = context.deltaTime;
	localContext.targetActor = actorPin->getRef();
	bool success = StateMachine::process(localContext);
	AnimationState* state = dynamic_cast<AnimationState*>(currentState);
	if (state)
		posePin->setPose(state->getPose());
	return success;
}

Serializable* AnimationStateMachine::instantiate(const SerializationInfo& from)
{
	return new AnimationStateMachine();
}
