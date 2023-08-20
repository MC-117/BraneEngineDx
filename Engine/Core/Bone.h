#pragma once
#ifndef _BONE_H_
#define _BONE_H_

#include "Actor.h"
#include "MeshRender.h"

class Bone : public Actor
{
	friend class Skeleton;
public:
	Serialize(Bone, Actor);

	Bone(const string& name = "Bone");

	int getBoneIndex() const;

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	Bone(int boneIndex, const string& name = "Bone");
	int boneIndex = -1;
};

#endif // !_BONE_H_
