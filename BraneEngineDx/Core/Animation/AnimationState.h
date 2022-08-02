#pragma once

#include "../Actor.h"
#include "AnimationPoseGraph.h"
#include "../Graph/StateMachine/StateNode.h"

class AnimationGraphContext : public GraphContext
{
public:
	SkeletonMeshActor* targetActor = NULL;

	virtual ~AnimationGraphContext() = default;
};

class AnimationStateContext : public GraphContext
{
public:
	float timeRemain = 0;
	float normalizedTimeRemain = 0;
	float duration = 0;

	virtual ~AnimationStateContext() = default;
};

class AnimationDefaultCanTransitionNode : public GraphNode
{
public:
	Serialize(AnimationDefaultCanTransitionNode, GraphNode);

	AnimationDefaultCanTransitionNode();
	virtual ~AnimationDefaultCanTransitionNode() = default;

	BoolPin* getOutputPin();

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	BoolPin* outputPin = NULL;
};

class AnimationTransition : public StateGraphTransition
{
public:
	Serialize(AnimationTransition, StateGraphTransition);

	AnimationTransition();

	void setDuration(float duration);
	float getDuration() const;

	virtual bool completeTransition() const;

	virtual void onBeginTransition(GraphContext& context);
	virtual void onTransition(GraphContext& context);
	virtual void onEndTransition(GraphContext& context);

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	float duration = 0.2f;
	float passedTime = 0;
	unsigned long long lastFrame = 0;

	FloatVariable* durationVariable;
	FloatVariable* timeRemainVariable;
	FloatVariable* normalizedTimeRemainVariable;
};

class AnimationState : public StateNode
{
public:
	Serialize(AnimationState, StateNode);

	AnimationState() = default;
	virtual ~AnimationState() = default;

	virtual void setName(const string& name);

	AnimationPoseGraph* getGraph();
	const AnimationPoseGraph* getGraph() const;

	virtual AnimationPose* getPose();

	virtual void onUpdateState(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	AnimationPoseGraph poseGraph;
};