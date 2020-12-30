#pragma once
#ifndef _ANIMATIONCLIP_H_

#include "Utility.h"
#include "SkeletonPose.h"

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
	bool update(float deltaTime);
	bool isLoop();
	void setLoop(bool loop);
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

	bool canApply(SkeletonData& skeletonData);
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
	void write(ostream& out) const;
};

class AnimationBase
{
public:
	virtual ~AnimationBase() {}
	virtual map<unsigned int, float>* getCurveCurrentValue() { return NULL; };
	virtual void mapCurveChannel(const vector<string>& names) { };
	virtual bool update(float deltaTime) = 0;
	virtual bool isLoop() = 0;
	virtual void setLoop(bool loop) = 0;
	virtual void play() = 0;
	virtual void pause() = 0;
	virtual void stop() = 0;
	virtual bool playing() = 0;
	virtual void setSpeed(float speed) = 0;
	virtual void reset() = 0;
};

class AnimationClip : public AnimationBase
{
public:
	AnimationClipData* animationClipData = NULL;
	vector<::TransformAnimationPlayer> animationPlayer;
	map<unsigned int, CurvePlayer<float, float>> curvePlayer;
	map<unsigned int, float> curveCurrentValue;
	map<unsigned int, float> curveCurrentValueMaped;
	bool useMapedCurve = false;
	// Map animation curve index to named channel index
	map<unsigned int, unsigned int> curveChannelMap;
public:
	AnimationClip();
	AnimationClip(AnimationClipData& clipData);

	virtual map<unsigned int, float>* getCurveCurrentValue();
	virtual void mapCurveChannel(const vector<string>& names);
	virtual void setAnimationClipData(AnimationClipData& clipData);
	virtual bool setTargetTransform(Transform& target);
	virtual bool setTargetTransform(const string& name, Transform& target);
	virtual bool update(float deltaTime);
	virtual void getTransformData(vector<TransformData>& data);
	virtual TransformData* getTransformData(const string& name);
	virtual bool isLoop();
	virtual void setLoop(bool loop);
	virtual void play();
	virtual void pause();
	virtual void stop();
	virtual bool playing();
	virtual void setSpeed(float speed);
	virtual void reset();
};

class BlendSpaceAnimation : public AnimationBase, public Serializable
{
public:
	Serialize(BlendSpaceAnimation);

	SkeletonData* skeletonData = NULL;
	Vector2f blendWeight;
	vector<pair<unsigned int, float>> animationWeights;
	vector<Transform*> targetTransforms;
	vector<TransformData> transformDatas;
	vector<pair<Vector2f, AnimationClip*>> animationClipWrap;

	BlendSpaceAnimation();
	BlendSpaceAnimation(SkeletonData& skeletonData);

	virtual bool isValid();

	virtual bool setSkeletonData(SkeletonData& skeletonData);
	virtual bool addAnimationClipData(const Vector2f& pos, AnimationClipData& clipData);
	virtual bool removeAnimationClipData(unsigned int index);
	virtual void removeAllAnimationClipData();
	virtual bool setTargetTransform(Transform& target);
	virtual bool setTargetTransform(const string& name, Transform& target);
	virtual void setBlendWeight(const Vector2f& weight);
	virtual unsigned int getWeights(vector<pair<unsigned int, float>>& weights);
	virtual bool update(float deltaTime);
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
	static bool writeAnimation(const AnimationClipData data, const string & file);
};

#endif // !_ANIMATIONCLIP_H_
