#pragma once
#ifndef _RIGIDBODY_H_
#define _RIGIDBODY_H_

#include "PhysicalBody.h"

enum DRIVABLE
{
DRIVABLE_SURFACE = 0xffff0000,
UNDRIVABLE_SURFACE = 0x0000ffff
};

enum VEHICLE_COLLISION_FLAG
{
	COLLISION_FLAG_GROUND = 1 << 0,
	COLLISION_FLAG_WHEEL = 1 << 1,
	COLLISION_FLAG_CHASSIS = 1 << 2,
	COLLISION_FLAG_OBSTACLE = 1 << 3,
	COLLISION_FLAG_DRIVABLE_OBSTACLE = 1 << 4,

	COLLISION_FLAG_GROUND_AGAINST = COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_WHEEL_AGAINST = COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE,
	COLLISION_FLAG_CHASSIS_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_OBSTACLE_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_DRIVABLE_OBSTACLE_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE
};

enum SURFACE_TYPE
{
	SURFACE_TYPE_TARMAC,
	MAX_NUM_SURFACE_TYPES
};

//Tire types.
enum TIRE_TYPE
{
	TIRE_TYPE_NORMAL = 0,
	//TIRE_TYPE_WORN,
	MAX_NUM_TIRE_TYPES
};

class RigidBodyCollider : public PhysicalCollider
{
public:
	PxShape* rawShape = NULL;
	CollisionShape* collisionShape = NULL;
	PxMaterial* rawMaterial = NULL;

	RigidBodyCollider(Shape* shape, ShapeComplexType shapeComplexType = SIMPLE, Vector3f scale = Vector3f::Ones());
	virtual ~RigidBodyCollider();

	virtual void setPositionOffset(const Vector3f& offset);
	virtual void setRotationOffset(const Quaternionf& offset);

	virtual void setLayer(const PhysicalLayer& layer);

	PxShape* getRawShape(PxPhysics& physics);
	virtual void apply();
	PTransform getOffsetTransform() const;
};

class RigidBody : public PhysicalBody
#ifdef PHYSICS_USE_BULLET
	, public CollisionRigidBody
#endif // !PHYSICS_USE_BULLET
{
public:
	Serialize(RigidBody, PhysicalBody);

	RigidBody(::Transform& targetTransform, const PhysicalMaterial& material);
	virtual ~RigidBody();

	virtual void initBody();
	virtual PhysicalCollider* addCollider(Shape* shape, ShapeComplexType shapeComplexType = SIMPLE);
	virtual void updateObjectTransform();
	virtual void setWorldTransform(const Vector3f& position, const Quaternionf& rotation);
	virtual void addToWorld(PhysicalWorld& physicalWorld);
	virtual void removeFromWorld();

	void setMass(float mass);
	virtual void setLockFlags(Enum<LockFlag> flags);

	virtual CollisionObject* getCollisionObject() const;

	void addForce(const Vector3f& force, bool autoWake = true);
	void addImpulse(const Vector3f& impulse, bool autoWake = true);
	void addAcceleration(const Vector3f& acceleration, bool autoWake = true);
	void addVelocity(const Vector3f& velocity, bool autoWake = true);

	void addForceAtLocation(const Vector3f& force, const Vector3f& location, bool autoWake = true);
	void addImpulseAtLocation(const Vector3f& impulse, const Vector3f& location, bool autoWake = true);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
#ifdef PHYSICS_USE_PHYSX
	PxRigidActor* rawRigidActor = NULL;
	PxRigidDynamic* rawRigidDynamic = NULL;
	PxRigidStatic* rawRigidStatic = NULL;
#endif // !PHYSICS_USE_PHYSX
};

#endif // !_RIGIDBODY_H_
