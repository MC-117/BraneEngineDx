#pragma once
#ifndef _SKELETONPOSE_H_
#define _SKELETONPOSE_H_

#include "Transform.h"
#include "SkeletonMesh.h"

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

struct SkeletonPose
{
	SkeletonData* skeletonData;
	vector<TransformData> transformData;

	bool isOperationValid(const SkeletonPose& pose);

	bool lerp(const SkeletonPose& pose, float value);
	bool lerpAbove(const SkeletonPose& pose, const string& name, float value, bool includeSelf = false);
	bool makeAddition(const SkeletonPose& pose, float value);
	bool makeAdditionAbove(const SkeletonPose& pose, const string& name, float value, bool includeSelf = false);
	bool applyAddition(const SkeletonPose& pose, float value);
	bool applyAdditionAbove(const SkeletonPose& pose, const string& name, float value, bool includeSelf = false);
};

#endif // !_SKELETONPOSE_H_
