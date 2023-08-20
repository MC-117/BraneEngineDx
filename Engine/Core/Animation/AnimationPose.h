#pragma once
#ifndef _SKELETONPOSE_H_
#define _SKELETONPOSE_H_

#include "../Skeleton/Skeleton.h"
#include "../MorphTargetWeight.h"

class Transform;

struct AnimationContext
{
	Skeleton* skeleton = NULL;
	MorphTargetRemapper* morphTargetRemapper = NULL;
};

enum struct AnimationUpdateFlags : unsigned char
{
	None = 0, Pos = 1, Rot = 2, Sca = 4, Trans = 7, Morph = 8, All = 15
};

enum struct AnimationBlendMode : unsigned char
{
	Replace, Additive
};

struct TransformData
{
	Vector3f position = { 0, 0, 0 };
	Quaternionf rotation = Quaternionf::Identity();
	Vector3f scale = { 1, 1, 1 };

	TransformData();
	TransformData(const TransformData& data);

	TransformData& operator=(const TransformData& data);

	TransformData& lerp(const TransformData& data, float value);
	TransformData& add(const TransformData& data);
	TransformData& mutiply(float value);
};

struct AnimationPose
{
	Skeleton* skeleton;
	MorphTargetRemapper* morphTargetRemapper;
	vector<TransformData> transformData;
	vector<float> morphTargetWeight;

	void setContext(const AnimationContext& context);
	void setSkeleton(Skeleton* data);
	void setMorphTargetMap(MorphTargetRemapper* mapper);

	bool isOperationValid(const AnimationPose& pose);

	void applyPose(Enum<AnimationUpdateFlags> flags);
	void applyPoseAdditive(const AnimationPose& bindPose, Enum<AnimationUpdateFlags> flags);

	bool resetToBindPose();

	bool lerp(const AnimationPose& pose, float value);
	bool lerpAbove(const AnimationPose& pose, const string& name, float value, bool includeSelf = false);
	bool makeAddition(const AnimationPose& pose);
	bool makeAdditionAbove(const AnimationPose& pose, const string& name, bool includeSelf = false);
	bool applyAddition(const AnimationPose& pose);
	bool applyAdditionAbove(const AnimationPose& pose, const string& name, bool includeSelf = false);
};

#endif // !_SKELETONPOSE_H_
