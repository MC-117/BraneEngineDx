#pragma once

#include "../Utility/Utility.h"
#include "AnimationPose.h"

struct TransformAnimationData
{
	string objectName;
	Curve<float, Vector3f> positionCurve;
	Curve<float, Quaternionf> rotationCurve;
	Curve<float, Vector3f> scaleCurve;

	void read(istream& in);
	void write(ostream& out) const;
};

class TransformAnimationPlayer
{
public:

	Transform* targetTransform = NULL;
	TransformAnimationData* transformAnimationData = NULL;
	CurvePlayer<float, Vector3f> positionCurvePlayer;
	CurvePlayer<float, Quaternionf> rotationCurvePlayer;
	CurvePlayer<float, Vector3f> scaleCurvePlayer;

	TransformAnimationPlayer();
	TransformAnimationPlayer(Transform& target);
	TransformAnimationPlayer(Transform& target, TransformAnimationData& animationData);
	void setTargetTransform(Transform& target);
	void setAnimationData(TransformAnimationData& animationData);
	TransformData& getTransformData();
	TransformData getTransformData(float timeRate);
	Enum<AnimationUpdateFlags> getUpdateFlags() const;
	Enum<AnimationUpdateFlags> update(float deltaTime);
	bool isLoop();
	void setLoop(bool loop);
	void setTime(float time);
	void play();
	void pause();
	void stop();
	bool playing();
	void setSpeed(float speed);
	void reset();
protected:
	TransformData transformData;
};

struct AnimationClipData
{
	string name;
	float duration = 0;
	map<string, unsigned int> transformAnimationNames;
	vector<TransformAnimationData> transformAnimationDatas;
	map<string, unsigned int> curveNames;
	map<unsigned int, Curve<float, float>> curves;

	AnimationClipData(const string& name);

	bool canApply(Skeleton& skeleton);
	bool canBlend(AnimationClipData& data);

	TransformAnimationData& addAnimatinData(const string& objectName);
	TransformAnimationData* getAnimatinData(const string& objectName);
	unsigned int getAnimatinDataIndex(const string& objectName);
	TransformAnimationData* getAnimatinData(unsigned int index);
	Curve<float, float>& addMorphAnimationData(unsigned int index);
	Curve<float, float>& addMorphAnimationData(const string& name);
	Curve<float, float>* getMorphAnimationData(unsigned int index);
	Curve<float, float>* getMorphAnimationData(const string& name);
	void pack();

	bool read(istream& in);
	void write(ostream& out, LongProgressWork* work) const;
};

class AnimationBase : public Serializable
{
public:
	Serialize(AnimationBase,);

	AnimationPose animationPose;
	Enum<AnimationUpdateFlags> updateFlags;
	virtual ~AnimationBase() = default;
	virtual bool setupContext(const AnimationContext& context);
	AnimationPose& getPose();
	virtual bool setTargetTransform(Transform& target) { return false; }
	virtual Enum<AnimationUpdateFlags> update(float deltaTime) = 0;
	virtual Enum<AnimationUpdateFlags> getUpdateFlags() const;
	virtual bool isValid() = 0;
	virtual bool isLoop() = 0;
	virtual void setLoop(bool loop) = 0;
	virtual void play() = 0;
	virtual void pause() = 0;
	virtual void stop() = 0;
	virtual bool playing() = 0;
	virtual void setSpeed(float speed) = 0;
	virtual void reset() = 0;

	static Serializable* instantiate(const SerializationInfo& from) { return NULL; }
	virtual bool deserialize(const SerializationInfo& from) { return true; }
	virtual bool serialize(SerializationInfo& to) { serializeInit(this, to); return true; }
};

class AnimationClip : public AnimationBase
{
public:
	AnimationClipData* animationClipData = NULL;
	vector<int> boneChannelMap;
	vector<int> curveChannelMap;
	vector<::TransformAnimationPlayer> animationPlayer;
	map<unsigned int, CurvePlayer<float, float>> curvePlayer;
	map<unsigned int, float> curveCurrentValue;
	AnimationPose bindPose;

	AnimationClip();
	AnimationClip(AnimationClipData& clipData);

	virtual bool isValid();

	virtual bool setupDefault();
	virtual bool setupContext(const AnimationContext& context);

	void remapChannel();

	virtual void setAnimationClipData(AnimationClipData* clipData);
	virtual bool setTargetTransform(Transform& target);
	virtual bool setTargetTransform(const string& name, Transform& target);
	virtual Enum<AnimationUpdateFlags> update(float deltaTime);
	virtual void getTransformData(vector<TransformData>& data);
	virtual TransformData* getTransformData(const string& name);
	virtual bool isLoop();
	virtual void setLoop(bool loop);
	virtual void setTime(float time);
	virtual float getTime() const;
	virtual void play();
	virtual void pause();
	virtual void stop();
	virtual bool playing();
	virtual void setSpeed(float speed);
	virtual void reset();
};

class Skeleton;
class BlendSpaceAnimation : public AnimationBase
{
public:
	Serialize(BlendSpaceAnimation, AnimationBase);

	string name;
	Vector2f blendWeight;
	vector<pair<unsigned int, float>> animationWeights;
	vector<Transform*> targetTransforms;
	vector<TransformData> transformDatas;
	vector<pair<Vector2f, AnimationClip*>> animationClipWrap;

	BlendSpaceAnimation();

	virtual bool isValid();

	virtual bool setupContext(const AnimationContext& context);
	virtual bool addAnimationClipData(const Vector2f& pos, AnimationClipData& clipData);
	virtual bool removeAnimationClipData(unsigned int index);
	virtual void removeAllAnimationClipData();
	virtual void setBlendWeight(const Vector2f& weight);
	virtual unsigned int getWeights(vector<pair<unsigned int, float>>& weights);
	virtual Enum<AnimationUpdateFlags> update(float deltaTime);
	virtual vector<TransformData>& getTransformData();
	virtual bool isLoop();
	virtual void setLoop(bool loop);
	virtual void play();
	virtual void pause();
	virtual void stop();
	virtual bool playing();
	virtual void setSpeed(float speed);
	virtual void reset();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

class AnimationLoader
{
public:
	static AnimationClipData * loadCameraAnimation(const string & file);
	static AnimationClipData * loadMotionAnimation(const string & file);
	static AnimationClipData * readAnimation(const string & file);
	static bool writeAnimation(const AnimationClipData& data, const string & file, LongProgressWork* work = NULL);
};
