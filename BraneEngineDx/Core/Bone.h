#pragma once
#ifndef _BONE_H_
#define _BONE_H_

#include "Actor.h"
#include "MeshRender.h"

class Bone : public Actor
{
public:
	Serialize(Bone, Actor);

	Bone(const string& name = "Bone");

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

#endif // !_BONE_H_
