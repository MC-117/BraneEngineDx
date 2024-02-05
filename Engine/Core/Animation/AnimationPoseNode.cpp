#include "AnimationPoseNode.h"
#include "../Asset.h"
#include "../SkeletonMeshActor.h"
#include "../Attributes/TagAttribute.h"

SerializeInstance(AnimationPosePin, DEF_ATTR(Tag, "Animation"));

AnimationPosePin::AnimationPosePin(const string& name) : ValuePin(name)
{
}

GraphPinShape AnimationPosePin::getPinShape() const
{
    return GraphPinShape::Diamond;
}

void AnimationPosePin::resetToDefault()
{
    pose = NULL;
}

void AnimationPosePin::assign(const ValuePin* other)
{
    const AnimationPosePin* pin = dynamic_cast<const AnimationPosePin*>(other);
    if (pin) {
        setPose(pin->getPose());
    }
}

void AnimationPosePin::castFrom(const ValuePin* other)
{
    const AnimationPosePin* pin = dynamic_cast<const AnimationPosePin*>(other);
    if (pin) {
        setPose(pin->getPose());
    }
}

bool AnimationPosePin::process(GraphContext& context)
{
    if (!isOutput) {
        GraphPin* pin = connectedPin;
        if (pin == NULL)
            pose = NULL;
        else
            pose = ((AnimationPosePin*)pin)->getPose();
    }
    return true;
}

void AnimationPosePin::setPose(AnimationPose* pose)
{
    this->pose = pose;
}

AnimationPose* AnimationPosePin::getPose() const
{
    return pose;
}

void AnimationPosePin::setContext(const AnimationContext& context)
{
    animationContext = context;
}

const AnimationContext& AnimationPosePin::getContext() const
{
    return animationContext;
}

Name AnimationPosePin::getVariableType() const
{
    static const Name varType = "AnimationPose";
    return varType;
}

bool AnimationPosePin::generateDefaultVariable(GraphCodeGenerationContext& context)
{
    return ValuePin::generateDefaultVariable(context);
}

Serializable* AnimationPosePin::instantiate(const SerializationInfo& from)
{
    return new AnimationPosePin(from.name);
}

SerializeInstance(AnimationPoseSourceNode, DEF_ATTR(Tag, "Animation"));

AnimationPoseSourceNode::AnimationPoseSourceNode() : GraphNode(), actorPin("Target"), poseOutputPin("Out")
{
    displayName = "SourcePose";
    flag = Flag::Expression;
    addInternalInput(actorPin);
    addInternalOutput(poseOutputPin);
}

string AnimationPoseSourceNode::getAnimationName() const
{
    return string();
}

void AnimationPoseSourceNode::setContext(const AnimationContext& context)
{
    pose.setContext(context);
    if (this->context.skeleton != context.skeleton) {
        pose.resetToBindPose();
        bindPose = pose;
    }
    this->context = context;
}

AnimationPose* AnimationPoseSourceNode::getPose()
{
    return &pose;
}

SkeletonMeshActorRefPin& AnimationPoseSourceNode::getPoseActorPin()
{
    return actorPin;
}

AnimationPosePin& AnimationPoseSourceNode::getPoseOutputPin()
{
    return poseOutputPin;
}

bool AnimationPoseSourceNode::process(GraphContext& context)
{
    SkeletonMeshActor* actor = actorPin.getRef();
    if (actor)
        setContext({ &actor->skeleton, &actor->morphTargetRemapper });
    poseOutputPin.setPose(&pose);
    return true;
}

Serializable* AnimationPoseSourceNode::instantiate(const SerializationInfo& from)
{
    return new AnimationPoseSourceNode();
}

bool AnimationPoseSourceNode::deserialize(const SerializationInfo& from)
{
    return GraphNode::deserialize(from);
}

bool AnimationPoseSourceNode::serialize(SerializationInfo& to)
{
    return GraphNode::serialize(to);
}

SerializeInstance(AnimationPoseResultNode, DEF_ATTR(Tag, "Animation"));

AnimationPoseResultNode::AnimationPoseResultNode() : ReturnNode(), poseInputPin("Result")
{
    displayName = "PoseResult";
    addInternalInput(poseInputPin);
}

AnimationPose* AnimationPoseResultNode::getPose() const
{
    return poseInputPin.getPose();
}

AnimationPosePin& AnimationPoseResultNode::getPoseInputPin()
{
    return poseInputPin;
}

Serializable* AnimationPoseResultNode::instantiate(const SerializationInfo& from)
{
    return new AnimationPoseResultNode();
}

bool AnimationPoseResultNode::deserialize(const SerializationInfo& from)
{
    return GraphNode::deserialize(from);
}

bool AnimationPoseResultNode::serialize(SerializationInfo& to)
{
    return GraphNode::serialize(to);
}

SerializeInstance(AnimationPoseApplyNode, DEF_ATTR(Tag, "Animation"));

AnimationPoseApplyNode::AnimationPoseApplyNode() : poseInputPin("Pose")
{
    displayName = "ApplyPose";
    addInternalInput(poseInputPin);
}

AnimationPose* AnimationPoseApplyNode::getPose() const
{
    return poseInputPin.getPose();
}

AnimationPosePin& AnimationPoseApplyNode::getPoseInputPin()
{
    return poseInputPin;
}

bool AnimationPoseApplyNode::process(GraphContext& context)
{
    AnimationPose* pose = getPose();
    if (pose)
        pose->applyPose(AnimationUpdateFlags::All);
    return true;
}

Serializable* AnimationPoseApplyNode::instantiate(const SerializationInfo& from)
{
    return new AnimationPoseApplyNode();
}

SerializeInstance(AnimationBaseNode);

void AnimationBaseNode::setContext(const AnimationContext& context)
{
    AnimationPoseSourceNode::setContext(context);
    animationBase.setupContext(context);
}

void AnimationBaseNode::setPlaying(bool playing)
{
    if (playing)
        animationBase.play();
    else
        animationBase.pause();
}

bool AnimationBaseNode::getPlaying() const
{
    return animationBase.playing();
}

void AnimationBaseNode::setLoop(bool loop)
{
    animationBase.setLoop(loop);
}

bool AnimationBaseNode::getLoop() const
{
    return animationBase.isLoop();
}

void AnimationBaseNode::setAutoPlay(bool autoPlay)
{
    this->autoPlay = autoPlay;
}

bool AnimationBaseNode::getAutoPlay() const
{
    return autoPlay;
}

float AnimationBaseNode::getTime() const
{
    return 0.0f;
}

float AnimationBaseNode::getNormalizedTime() const
{
    return 0.0f;
}

void AnimationBaseNode::setBlendMode(AnimationBlendMode mode)
{
    blendMode = mode;
}

AnimationBlendMode AnimationBaseNode::getBlendMode() const
{
    return blendMode;
}

bool AnimationBaseNode::process(GraphContext& context)
{
    if (!AnimationPoseSourceNode::process(context))
        return false;
    if (Time::frames() <= processedFrame)
        return true;
    processedFrame = Time::frames();
    if (autoPlay && !animationBase.playing())
        animationBase.play();
    animationBase.update(context.deltaTime);
    switch (blendMode)
    {
    case AnimationBlendMode::Replace:
        pose = animationBase.getPose();
        break;
    case AnimationBlendMode::Additive:
        pose = bindPose;
        pose.applyAddition(animationBase.getPose());
        break;
    default:
        break;
    }
    timePin.setValue(getTime());
    timeNormalizedPin.setValue(getNormalizedTime());
    return true;
}

Serializable* AnimationBaseNode::instantiate(const SerializationInfo& from)
{
    return nullptr;
}

bool AnimationBaseNode::deserialize(const SerializationInfo& from)
{
    if (!AnimationPoseSourceNode::deserialize(from))
        return false;
    string autoPlayStr;
    if (from.get("autoPlay", autoPlayStr)) {
        autoPlay = autoPlayStr == "true";
    }
    return true;
}

bool AnimationBaseNode::serialize(SerializationInfo& to)
{
    if (!AnimationPoseSourceNode::serialize(to))
        return false;
    to.add("autoPlay", autoPlay ? "true" : "false");
    return true;
}

AnimationBaseNode::AnimationBaseNode(AnimationBase& animationBase, const string& name)
    : AnimationPoseSourceNode(), animationBase(animationBase),
    timePin("Time"), timeNormalizedPin("NormTime")
{
    addInternalOutput(timePin);
    addInternalOutput(timeNormalizedPin);
}

SerializeInstance(AnimationClipNode);

AnimationClipNode::AnimationClipNode(const string& name)
    : AnimationBaseNode(clip, name), updatePin("Update"), endPin("End")
{
    displayName = "AnimationClip";
    addInternalOutput(updatePin);
    addInternalOutput(endPin);
}

string AnimationClipNode::getAnimationName() const
{
    return clip.animationClipData ? clip.animationClipData->name : "";
}

void AnimationClipNode::setContext(const AnimationContext& context)
{
    AnimationBaseNode::setContext(context);
}

void AnimationClipNode::setAnimation(AnimationClipData* data)
{
    clip.setAnimationClipData(data);
}

AnimationClipData* AnimationClipNode::getAnimation() const
{
    return clip.animationClipData;
}

AnimationClip& AnimationClipNode::getAnimationClip()
{
    return clip;
}

float AnimationClipNode::getTime() const
{
    return clip.getTime();
}

float AnimationClipNode::getNormalizedTime() const
{
    if (clip.animationClipData)
        return clip.getTime() / clip.animationClipData->duration;
    return 0;
}

bool AnimationClipNode::flowControl(GraphContext& context)
{
    bool success = poseOutputPin.process(context);
    return success;
}

bool AnimationClipNode::process(GraphContext& context)
{
    if (Time::frames() <= processedFrame)
        return true;
    float normalizedTime = getNormalizedTime();
    if (normalizedTime == 1 ||
        normalizedTime < lastNormalizedTime)
        endPin.process(context);
    lastNormalizedTime = normalizedTime;
    updatePin.process(context);
    return AnimationBaseNode::process(context);
}

Serializable* AnimationClipNode::instantiate(const SerializationInfo& from)
{
    return new AnimationClipNode(from.name);
}

bool AnimationClipNode::deserialize(const SerializationInfo& from)
{
    if (!AnimationBaseNode::deserialize(from))
        return false;
    string path;
    from.get("clipPath", path);
    AnimationClipData* data = getAssetByPath<AnimationClipData>(path);
    setAnimation(data);
    return true;
}

bool AnimationClipNode::serialize(SerializationInfo& to)
{
    if (!AnimationBaseNode::serialize(to))
        return false;
    string path = AnimationClipDataAssetInfo::getPath(getAnimation());
    to.set("clipPath", path);
    return true;
}

SerializeInstance(BlendSpaceNode);

BlendSpaceNode::BlendSpaceNode(const string& name)
    : AnimationBaseNode(blendSpace, name), xPin("X"), yPin("Y")
{
    displayName = "BlendSpace";
}

string BlendSpaceNode::getAnimationName() const
{
    return blendSpace.name;
}

void BlendSpaceNode::setContext(const AnimationContext& context)
{
    AnimationBaseNode::setContext(context);
}

BlendSpaceAnimation& BlendSpaceNode::getBlendSpace()
{
    return blendSpace;
}

bool BlendSpaceNode::process(GraphContext& context)
{
    blendSpace.setBlendWeight({ xPin.getValue(), yPin.getValue() });
    return AnimationBaseNode::process(context);
}

Serializable* BlendSpaceNode::instantiate(const SerializationInfo& from)
{
    return new BlendSpaceNode(from.name);
}

bool BlendSpaceNode::deserialize(const SerializationInfo& from)
{
    if (!AnimationBaseNode::deserialize(from))
        return false;
    string path;
    const SerializationInfo* info = from.get("blendSpace");
    if (info != NULL) {
        blendSpace.deserialize(*info);
    }
    return true;
}

bool BlendSpaceNode::serialize(SerializationInfo& to)
{
    if (!AnimationBaseNode::serialize(to))
        return false;
    SerializationInfo* info = to.add("blendSpace");
    if (info != NULL) {
        blendSpace.serialize(*info);
    }
    return true;
}

SerializeInstance(AnimationPoseLerpNode, DEF_ATTR(Tag, "Animation"));

AnimationPoseLerpNode::AnimationPoseLerpNode() : poseAPin("PoseA"), poseBPin("PoseB"), valuePin("Value"), poseOutPin("Out")
{
    displayName = "LerpPose";
    flag = Flag::Expression;
    addInternalInput(poseAPin);
    addInternalInput(poseBPin);
    addInternalInput(valuePin);
    addInternalOutput(poseOutPin);
}

void AnimationPoseLerpNode::setContext(const AnimationContext& context)
{
}

AnimationPose* AnimationPoseLerpNode::getPose()
{
    return poseAPin.getPose();
}

bool AnimationPoseLerpNode::process(GraphContext& context)
{
    AnimationPose* poseA = poseAPin.getPose();
    AnimationPose* poseB = poseBPin.getPose();
    if (poseA && poseB) {
        float value = valuePin.getValue();
        poseA->lerp(*poseB, value);
    }
    poseOutPin.setPose(poseA);
    return true;
}

Serializable* AnimationPoseLerpNode::instantiate(const SerializationInfo& from)
{
    return new AnimationPoseLerpNode();
}
