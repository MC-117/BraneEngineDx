#include "AnimationPoseGraph.h"
#include "../SkeletonMeshActor.h"

SerializeInstance(AnimationPoseGraph, DEF_ATTR(Namespace, "Animation"));

AnimationPoseGraph::AnimationPoseGraph()
{
    flag = Flag::Expression;
    actorPin = new SkeletonMeshActorRefPin("Target");
    posePin = new AnimationPosePin("Result");
    addParameter(actorPin);
    addReturn(posePin);
    setEntryNode();
    poseNode = new AnimationPoseResultNode();
    nodes.push_back(poseNode);
    returnNodes.push_back(poseNode);
    timeVariable = new FloatVariable("Time");
    normalizedTimeVariable = new FloatVariable("NormalizedTime");
    addVariable(timeVariable);
    addVariable(normalizedTimeVariable);
}

void AnimationPoseGraph::setContext(const AnimationContext& context)
{
    pose.setContext(context);
    if (animationContext.skeleton != context.skeleton) {
        pose.resetToBindPose();
    }
    animationContext = context;
}

void AnimationPoseGraph::setTargetActor(SkeletonMeshActor* actor)
{
    actorPin->setRef(actor);
}

AnimationPoseResultNode& AnimationPoseGraph::getResultState()
{
    return *poseNode;
}

AnimationPose* AnimationPoseGraph::getPose()
{
    return &pose;
}

float AnimationPoseGraph::getTime() const
{
    return timeVariable->getValue();
}

float AnimationPoseGraph::getNormalizedTime() const
{
    return normalizedTimeVariable->getValue();
}

bool AnimationPoseGraph::process(GraphContext& context)
{
    SkeletonMeshActor* actor = actorPin->getRef();
    if (actor)
        setContext({ &actor->skeleton, &actor->morphTargetRemapper });
    if (entryNode) {
		for (int i = 0; i < inputs.size(); i++) {
			ValuePin* input = dynamic_cast<ValuePin*>(getInput(i));
			ValuePin* output = dynamic_cast<ValuePin*>(entryNode->getOutput(i));
			if (input && output) {
				output->assign(input);
			}
		}
	}
    context.executeNode(poseNode);
    AnimationPose* resultPose = poseNode->getPose();
    if (resultPose != NULL)
        pose = *resultPose;
    posePin->setPose(&pose);
    return true;
}

Serializable* AnimationPoseGraph::instantiate(const SerializationInfo& from)
{
    return new AnimationPoseGraph();
}
