#include "AnimationPose.h"
#include "../Bone.h"

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
	float c = rotation.dot(data.rotation);
	if (c < 0.0f)
		(Vector4f&)rotation = -(Vector4f&)rotation;
	rotation = rotation.slerp(value, data.rotation);
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
	Quaternionf q0 = Quaternionf(1, 0, 0, 0);
	float c = q0.dot(rotation);
	if (c < 0.0f)
		(Vector4f&)rotation = -(Vector4f&)rotation;
	rotation = q0.slerp(value, rotation);
	scale *= value;
	return *this;
}

void AnimationPose::setContext(const AnimationContext& context)
{
	if (context.skeleton != NULL) {
		this->skeleton = context.skeleton;
		transformData.resize(context.skeleton->bones.size());
	}
	morphTargetRemapper = context.morphTargetRemapper;
	if (context.morphTargetRemapper == NULL) {
		morphTargetWeight.clear();
	}
	else {
		morphTargetWeight.resize(context.morphTargetRemapper->morphNames.size());
	}
}

void AnimationPose::setSkeleton(Skeleton* skeleton)
{
	this->skeleton = skeleton;
	transformData.resize(skeleton->bones.size());
}

void AnimationPose::setMorphTargetMap(MorphTargetRemapper* mapper)
{
	if (mapper != NULL)
		morphTargetWeight.resize(mapper->morphNames.size());
}

bool AnimationPose::isOperationValid(const AnimationPose & pose)
{
	return skeleton == pose.skeleton &&
		transformData.size() == pose.transformData.size() &&
		morphTargetWeight.size() == pose.morphTargetWeight.size();
}

void AnimationPose::applyPose(Enum<AnimationUpdateFlags> flags)
{
	if (skeleton != NULL && flags.intersect(AnimationUpdateFlags::Trans)) {
		for (int i = 0; i < transformData.size(); i++)
		{
			Skeleton::BoneInfo* info = skeleton->getBone(i);
			TransformData& data = transformData[i];
			if (info->bone != NULL) {
				if (flags.has(AnimationUpdateFlags::Pos))
					info->bone->setPosition(data.position);
				if (flags.has(AnimationUpdateFlags::Rot))
					info->bone->setRotation(data.rotation);
				if (flags.has(AnimationUpdateFlags::Sca))
					info->bone->setScale(data.scale);
			}
		}
	}
	if (morphTargetRemapper != NULL && flags.intersect(AnimationUpdateFlags::Morph)) {
		for (int i = 0; i < morphTargetWeight.size(); i++) {
			morphTargetRemapper->setMorphWeight(i, morphTargetWeight[i]);
		}
	}
}

void AnimationPose::applyPoseAdditive(const AnimationPose& bindPose, Enum<AnimationUpdateFlags> flags)
{
	if (skeleton != NULL && flags.intersect(AnimationUpdateFlags::Trans)) {
		for (int i = 0; i < transformData.size(); i++)
		{
			Skeleton::BoneInfo* info = skeleton->getBone(i);
			TransformData& data = transformData[i];
			const TransformData& bindData = bindPose.transformData[i];
			if (info->bone != NULL) {
				if (flags.has(AnimationUpdateFlags::Pos))
					info->bone->setPosition(data.position + bindData.position);
				if (flags.has(AnimationUpdateFlags::Rot))
					info->bone->setRotation(bindData.rotation * data.rotation);
				if (flags.has(AnimationUpdateFlags::Sca))
					info->bone->setScale(data.scale.cwiseProduct(bindData.scale));
			}
		}
	}
	if (morphTargetRemapper != NULL && flags.intersect(AnimationUpdateFlags::Morph)) {
		for (int i = 0; i < morphTargetWeight.size(); i++) {
			morphTargetRemapper->setMorphWeight(i, morphTargetWeight[i]);
		}
	}
}

bool AnimationPose::resetToBindPose()
{
	if (skeleton == NULL)
		return false;
	for (auto b = skeleton->boneNames.begin(),
		e = skeleton->boneNames.end(); b != e; b++) {
		TransformData& data = transformData[b->second];
		skeleton->bones[b->second]->data->transformMatrix.decompose(
			data.position, data.rotation, data.scale);
	}
	for (int i = 0; i < morphTargetWeight.size(); i++) {
		morphTargetWeight[i] = 0;
	}
	return true;
}

bool AnimationPose::lerp(const AnimationPose & pose, float value)
{
	if (!isOperationValid(pose))
		return false;
	for (int i = 0; i < transformData.size(); i++)
		transformData[i].lerp(pose.transformData[i], value);
	for (int i = 0; i < morphTargetWeight.size(); i++) {
		float& weight = morphTargetWeight[i];
		float otherWeight = pose.morphTargetWeight[i];
		weight = weight * (1 - value) + otherWeight * value;
	}
	return true;
}

bool AnimationPose::lerpAbove(const AnimationPose & pose, const string & name, float value, bool includeSelf)
{
	if (skeleton == NULL || !isOperationValid(pose))
		return false;
	Skeleton::BoneInfo* info = skeleton->getBone(name);
	if (info == NULL)
		return false;
	BoneData* rb = info->data;
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
	for (int i = 0; i < morphTargetWeight.size(); i++) {
		float& weight = morphTargetWeight[i];
		float otherWeight = pose.morphTargetWeight[i];
		weight = weight * (1 - value) + otherWeight * value;
	}
	return true;
}

bool AnimationPose::makeAddition(const AnimationPose & pose)
{
	return false;
}

bool AnimationPose::makeAdditionAbove(const AnimationPose & pose, const string & name, bool includeSelf)
{
	return false;
}

bool AnimationPose::applyAddition(const AnimationPose & pose)
{
	if (transformData.size() != pose.transformData.size())
		return false;
	for (int i = 0; i < transformData.size(); i++) {
		TransformData& data = transformData[i];
		const TransformData& bindData = pose.transformData[i];
		data.position += bindData.position;
		data.rotation = bindData.rotation * data.rotation;
		data.scale += bindData.scale;
	}
	return true;
}

bool AnimationPose::applyAdditionAbove(const AnimationPose & pose, const string & name, bool includeSelf)
{
	return false;
}