#pragma once
#ifndef _CHARACTER_H_
#define _CHARACTER_H_

#include "SkeletonMeshActor.h"
#include "Physics/PhysicalController.h"

class ENGINE_API Character : public Actor
{
public:
	Serialize(Character, Actor);

	PhysicalController physicalController;

	Character(Capsule capsule, string name = "Character");

	virtual void tick(float deltaTime);
	virtual void afterTick();

	virtual void setupPhysics(PhysicalWorld& physicalWorld);
	virtual void releasePhysics(PhysicalWorld& physicalWorld);

	virtual bool isFly();
	virtual void move(const Vector3f& v);
	virtual void jump(float impulse);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

#endif // !_CHARACTER_H_
