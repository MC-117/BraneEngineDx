#include "AnimationState.h"
#include "../Attributes/TagAttribute.h"

SerializeInstance(AnimationDefaultCanTransitionNode, DEF_ATTR(Tag, "Animation"));

AnimationDefaultCanTransitionNode::AnimationDefaultCanTransitionNode()
{
    flag = Flag::Expression;
    displayName = "DefaultCanTransition";
    outputPin = new BoolPin("CanTransition");
    addOutput(*outputPin);
}

BoolPin* AnimationDefaultCanTransitionNode::getOutputPin()
{
    return outputPin;
}

bool AnimationDefaultCanTransitionNode::process(GraphContext& context)
{
    bool canTransition = false;
    AnimationStateContext* animContext = dynamic_cast<AnimationStateContext*>(&context);
    if (animContext) {
        canTransition = animContext->timeRemain <= animContext->duration;
    }
    outputPin->setValue(canTransition);
    return false;
}

Serializable* AnimationDefaultCanTransitionNode::instantiate(const SerializationInfo& from)
{
    return new AnimationDefaultCanTransitionNode();
}

SerializeInstance(AnimationTransition, DEF_ATTR(Tag, "Animation"));

AnimationTransition::AnimationTransition()
{
    durationVariable = new FloatVariable("Duration");
    timeRemainVariable = new FloatVariable("TimeRemain");
    normalizedTimeRemainVariable = new FloatVariable("NormalizedTimeRemain");
    transitionGraph.addVariable(durationVariable);
    transitionGraph.addVariable(timeRemainVariable);
    transitionGraph.addVariable(normalizedTimeRemainVariable);
}

void AnimationTransition::setDuration(float duration)
{
    this->duration = max(duration, 0);
}

float AnimationTransition::getDuration() const
{
    return duration;
}

bool AnimationTransition::completeTransition() const
{
    return passedTime >= duration;
}

void AnimationTransition::onBeginTransition(GraphContext& context)
{
    passedTime = 0;
}

void AnimationTransition::onTransition(GraphContext& context)
{
    if (duration <= 0 || lastFrame >= Time::frames())
        return;
    AnimationState* animState = dynamic_cast<AnimationState*>(getState());
    AnimationState* nextAnimState = dynamic_cast<AnimationState*>(getNextState());
    if (animState && nextAnimState) {
        AnimationPose* pose = animState->getPose();
        AnimationPose* nextPose = nextAnimState->getPose();
        if (pose && nextPose) {
            pose->lerp(*nextPose, passedTime / duration);
        }
    }
    passedTime += context.deltaTime;
    lastFrame = Time::frames();
}

void AnimationTransition::onEndTransition(GraphContext& context)
{
    passedTime = 0;
}

bool AnimationTransition::process(GraphContext& context)
{
    if (!getNextState())
        return false;
    AnimationStateContext localContext;
    localContext.deltaTime = context.deltaTime;
    AnimationState* animState = dynamic_cast<AnimationState*>(getState());
    if (animState) {
        AnimationPoseGraph* poseGraph = animState->getGraph();
        if (poseGraph) {
            float time = poseGraph->getTime();
            float normalizedTime = poseGraph->getNormalizedTime();
            float normalizedTimeRemain = 1 - normalizedTime;
            float timeRemain = normalizedTime == 0 ? time :
                (time / normalizedTime * normalizedTimeRemain);
            timeRemainVariable->setValue(timeRemain);
            localContext.timeRemain = timeRemain;
            normalizedTimeRemainVariable->setValue(normalizedTimeRemain);
            localContext.normalizedTimeRemain = normalizedTimeRemain;
            durationVariable->setValue(duration);
            localContext.duration = duration;
        }
    }
    return transitionGraph.solveState(localContext);
}

Serializable* AnimationTransition::instantiate(const SerializationInfo& from)
{
    return new AnimationTransition();
}

bool AnimationTransition::deserialize(const SerializationInfo& from)
{
    if (!StateGraphTransition::deserialize(from))
        return false;
    from.get("duration", duration);
    return true;
}

bool AnimationTransition::serialize(SerializationInfo& to)
{
    if (!StateGraphTransition::serialize(to))
        return false;
    to.set("duration", duration);
    return true;
}

SerializeInstance(AnimationState, DEF_ATTR(Tag, "Animation"));

void AnimationState::setName(const Name& name)
{
    StateNode::setName(name);
    poseGraph.setName(name + "Graph");
}

AnimationPoseGraph* AnimationState::getGraph()
{
    return &poseGraph;
}

const AnimationPoseGraph* AnimationState::getGraph() const
{
    return &poseGraph;
}

AnimationPose* AnimationState::getPose()
{
    return poseGraph.getPose();
}

void AnimationState::onUpdateState(GraphContext& context)
{
    AnimationGraphContext localContext;
    localContext.deltaTime = context.deltaTime;
    localContext.targetActor = ((AnimationGraphContext&)context).targetActor;
    poseGraph.setTargetActor(localContext.targetActor);
    poseGraph.solveState(localContext);
}

Serializable* AnimationState::instantiate(const SerializationInfo& from)
{
    return new AnimationState();
}

bool AnimationState::deserialize(const SerializationInfo& from)
{
    if (!StateNode::deserialize(from))
        return false;
    const SerializationInfo* graphInfo = from.get("poseGraph");
    if (graphInfo)
        poseGraph.deserialize(*graphInfo);
    return true;
}

bool AnimationState::serialize(SerializationInfo& to)
{
    if (!StateNode::serialize(to))
        return false;
    SerializationInfo* graphInfo = to.add("poseGraph");
    if (graphInfo)
        poseGraph.serialize(*graphInfo);
    return true;
}
