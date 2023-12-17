#pragma once

#include "../Graph/ObjectNodes/SkeletonMeshActorNode.h"
#include "AnimationClip.h"

class ENGINE_API IAnimationPoseNode
{
public:
	virtual void setContext(const AnimationContext& context) = 0;
	virtual AnimationPose* getPose() = 0;
};

class ENGINE_API AnimationPosePin : public ValuePin
{
	friend class AnimationPoseSourceNode;
	friend class AnimationPoseResultNode;
public:
	Serialize(AnimationPosePin, ValuePin);

	AnimationPosePin(const string& name);

	virtual GraphPinShape getPinShape() const;

	virtual void resetToDefault();

	virtual void assign(const ValuePin* other);
	virtual void castFrom(const ValuePin* other);

	virtual bool process(GraphContext& context);

	void setPose(AnimationPose* pose);
	AnimationPose* getPose() const;

	void setContext(const AnimationContext& context);
	const AnimationContext& getContext() const;

	virtual Name getVariableType() const;
	virtual bool generateDefaultVariable(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	AnimationContext animationContext;
	AnimationPose* pose = NULL;
};

class ENGINE_API AnimationPoseSourceNode : public GraphNode, public IAnimationPoseNode
{
public:
	Serialize(AnimationPoseSourceNode, GraphNode);

	AnimationPoseSourceNode();
	virtual ~AnimationPoseSourceNode() = default;

	virtual string getAnimationName() const;

	virtual void setContext(const AnimationContext& context);

	virtual AnimationPose* getPose();
	SkeletonMeshActorRefPin& getPoseActorPin();
	AnimationPosePin& getPoseOutputPin();

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	AnimationContext context;
	SkeletonMeshActorRefPin actorPin;
	AnimationPosePin poseOutputPin;
	AnimationPose pose;
	AnimationPose bindPose;
};

class ENGINE_API AnimationPoseResultNode : public ReturnNode
{
public:
	Serialize(AnimationPoseResultNode, ReturnNode);

	AnimationPoseResultNode();
	virtual ~AnimationPoseResultNode() = default;

	virtual AnimationPose* getPose() const;
	AnimationPosePin& getPoseInputPin();

	static Serializable* instantiate(const SerializationInfo & from);
	virtual bool deserialize(const SerializationInfo & from);
	virtual bool serialize(SerializationInfo & to);
protected:
	AnimationPosePin poseInputPin;
};

class ENGINE_API AnimationPoseApplyNode : public InOutFlowNode
{
public:
	Serialize(AnimationPoseApplyNode, GraphNode);

	AnimationPoseApplyNode();
	virtual ~AnimationPoseApplyNode() = default;

	virtual AnimationPose* getPose() const;
	AnimationPosePin& getPoseInputPin();

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	AnimationPosePin poseInputPin;
};

class ENGINE_API AnimationBaseNode : public AnimationPoseSourceNode
{
public:
	Serialize(AnimationBaseNode, AnimationPoseSourceNode);

	virtual ~AnimationBaseNode() = default;

	virtual void setContext(const AnimationContext& context);

	void setPlaying(bool playing);
	bool getPlaying() const;

	void setLoop(bool loop);
	bool getLoop() const;

	void setAutoPlay(bool autoPlay);
	bool getAutoPlay() const;

	virtual float getTime() const;
	virtual float getNormalizedTime() const;

	void setBlendMode(AnimationBlendMode mode);
	AnimationBlendMode getBlendMode() const;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	FloatPin timePin;
	FloatPin timeNormalizedPin;
	AnimationBase& animationBase;
	AnimationBlendMode blendMode;
	bool autoPlay = true;
	unsigned long long processedFrame = 0;

	AnimationBaseNode(AnimationBase& animationBase, const string& name = "AnimationBase");
};

class ENGINE_API AnimationClipNode : public AnimationBaseNode
{
public:
	Serialize(AnimationClipNode, AnimationBaseNode);

	AnimationClipNode(const string& name = "Animation");
	virtual ~AnimationClipNode() = default;

	virtual string getAnimationName() const;

	virtual void setContext(const AnimationContext & context);

	void setAnimation(AnimationClipData* data);
	AnimationClipData* getAnimation() const;

	AnimationClip& getAnimationClip();

	virtual float getTime() const;
	virtual float getNormalizedTime() const;

	virtual bool flowControl(GraphContext& context);

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	AnimationClip clip;
	float lastNormalizedTime;

	FlowPin updatePin;
	FlowPin endPin;
};

class ENGINE_API BlendSpaceNode : public AnimationBaseNode
{
public:
	Serialize(BlendSpaceNode, AnimationBaseNode);

	BlendSpaceNode(const string& name = "BlendSpace");
	virtual ~BlendSpaceNode() = default;

	virtual string getAnimationName() const;

	virtual void setContext(const AnimationContext & context);

	BlendSpaceAnimation& getBlendSpace();

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	FloatPin xPin;
	FloatPin yPin;

	BlendSpaceAnimation blendSpace;
	AnimationBlendMode blendMode;
	vector<int> boneIndexMap;
	vector<int> morphChannelMap;
	vector<TransformData> bindPoses;
};

class ENGINE_API AnimationPoseLerpNode : public GraphNode, public IAnimationPoseNode
{
public:
	Serialize(AnimationPoseLerpNode, GraphNode);

	AnimationPoseLerpNode();
	virtual ~AnimationPoseLerpNode() = default;

	virtual void setContext(const AnimationContext& context);
	virtual AnimationPose* getPose();

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	AnimationPosePin poseAPin;
	AnimationPosePin poseBPin;
	FloatPin valuePin;
	AnimationPosePin poseOutPin;
};
