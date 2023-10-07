#pragma once

#include "BoneConstraint.h"

class Skeleton;
class Transform;

class ENGINE_API TwoBoneIKConstraint : public BoneConstraint
{
public:
	Serialize(TwoBoneIKConstraint, BoneConstraint);

	Ref<Bone> endBone;
	Ref<Bone> midBone;
	Ref<Bone> rootBone;
	Ref<Transform> poleTransform;
	Ref<Transform> effectTransform;
	Vector3f refAxis;
	Vector3f polePosition;
	Vector3f effectPosition;
	TransformSpace poleSpace = WORLD;
	TransformSpace effectSpace = WORLD;

	TwoBoneIKConstraint() = default;
	virtual bool isValid() const;
	virtual bool setup();
	virtual void solve();

	Vector3f getPoleWorldPosition() const;
	Vector3f getEffectWorldPosition() const;

	void setPoleWorldPosition(const Vector3f& pos);
	void setEffectWorldPosition(const Vector3f& pos);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};