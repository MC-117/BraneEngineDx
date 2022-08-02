#pragma once

#include "AnimationPoseNode.h"
#include "../Graph/Graph.h"

class AnimationPoseGraph : public Graph, public IAnimationPoseNode
{
public:
	Serialize(AnimationPoseGraph, Graph);

	AnimationPoseGraph();
	virtual ~AnimationPoseGraph() = default;

	virtual void setContext(const AnimationContext& context);

	void setTargetActor(SkeletonMeshActor* actor);

	AnimationPoseResultNode& getResultState();
	virtual AnimationPose* getPose();

	float getTime() const;
	float getNormalizedTime() const;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	AnimationContext animationContext;
	SkeletonMeshActorRefPin* actorPin;
	AnimationPosePin* posePin;
	AnimationPoseResultNode* poseNode;
	FloatVariable* timeVariable;
	FloatVariable* normalizedTimeVariable;
	AnimationPose pose;
};
