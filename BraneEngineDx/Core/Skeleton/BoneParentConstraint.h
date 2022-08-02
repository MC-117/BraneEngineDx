#pragma once

#include "BoneConstraint.h"

class BoneParentConstraint : public BoneConstraint
{
public:
	Serialize(BoneParentConstraint, BoneConstraint);

	enum Flags
	{
		None = 0, Position = 1, Rotation = 2
	};

	Ref<Bone> parentBone;
	Ref<Bone> childBone;
	Vector3f offsetPostion;
	Quaternionf offsetRotation;
	float weight = 1;
	Enum<Flags> flags;

	virtual bool isValid() const;
	virtual bool setup();
	virtual void solve();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};