#include "SkeletonPose.h"

TransformData::TransformData()
{
}

TransformData::TransformData(const TransformData & data)
{
	position = data.position;
	rotation = data.rotation;
	scale = data.scale;
}

TransformData & TransformData::operator=(const TransformData & data)
{
	position = data.position;
	rotation = data.rotation;
	scale = data.scale;
	return *this;
}

TransformData & TransformData::lerp(const TransformData & data, float value)
{
	position = position * (1 - value) + data.position * value;
	rotation.slerp(value, data.rotation);
	scale = scale * (1 - value) + data.scale * value;
	return *this;
}

TransformData & TransformData::add(const TransformData & data)
{
	position += data.position;
	rotation = data.rotation * rotation;
	scale += data.scale;
	return *this;
}

TransformData & TransformData::mutiply(float value)
{
	position *= value;
	rotation = Quaternionf(1, 0, 0, 0).slerp(value, rotation);
	scale *= value;
	return *this;
}

bool SkeletonPose::isOperationValid(const SkeletonPose & pose)
{
	return skeletonData == pose.skeletonData && transformData.size() == pose.transformData.size();
}

bool SkeletonPose::lerp(const SkeletonPose & pose, float value)
{
	if (!isOperationValid(pose))
		return false;
	for (int i = 0; i < transformData.size(); i++)
		transformData[i].lerp(pose.transformData[i], value);
	return true;
}

bool SkeletonPose::lerpAbove(const SkeletonPose & pose, const string & name, float value, bool includeSelf)
{
	if (skeletonData == NULL || !isOperationValid(pose))
		return false;
	BoneData* rb = skeletonData->getBoneData(name);
	if (rb == NULL)
		return false;
	BoneData* b;
	if (includeSelf)
		b = rb;
	else
		b = rb->getNext();
	while (b != NULL) {
		try {
			transformData[b->index].lerp(pose.transformData[b->index], value);
		}
		catch (exception e) {
			return false;
		}
		b = b->getNext(rb);
	}
	return true;
}

bool SkeletonPose::makeAddition(const SkeletonPose & pose, float value)
{
	return false;
}

bool SkeletonPose::makeAdditionAbove(const SkeletonPose & pose, const string & name, float value, bool includeSelf)
{
	return false;
}

bool SkeletonPose::applyAddition(const SkeletonPose & pose, float value)
{
	return false;
}

bool SkeletonPose::applyAdditionAbove(const SkeletonPose & pose, const string & name, float value, bool includeSelf)
{
	return false;
}