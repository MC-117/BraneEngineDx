#pragma once
#ifndef _PHYSICALBODY_H_
#define _PHYSICALBODY_H_

#include "../Transform.h"
#include "PhysicalMaterial.h"
#include "ContactInfo.h"
#include "PhysicalLayer.h"
#include "PhysicalBase.h"

class PhysicalWorld;

#ifdef PHYSICS_USE_BULLET
class PhysicalBodyMotionState : public btDefaultMotionState {
public:
	::Transform& targetTransform;

	PhysicalBodyMotionState(::Transform& targetTransform, const PTransform& centerOfMassOffset);

	virtual void getWorldTransform(PTransform& worldTrans) const;
	virtual void setWorldTransform(const PTransform& worldTrans);
};
#endif // PHYSICS_USE_BULLET

class PhysicalBody;

class PhysicalCollider
{
public:
	PhysicalBody* body = NULL;
	ShapeComplexType shapeComplexType = SIMPLE;
	Vector3f localScale = Vector3f::Ones();
	Shape* shape = NULL;

	PhysicalLayer layer;

	Vector3f positionOffset;
	Quaternionf rotationOffset = Quaternionf::Identity();

	PhysicalCollider(Shape* shape, ShapeComplexType shapeComplexType = SIMPLE, Vector3f scale = Vector3f::Ones());
	virtual ~PhysicalCollider();

	virtual void setPositionOffset(const Vector3f& offset);
	virtual Vector3f getPositionOffset();
	virtual void setRotationOffset(const Quaternionf& offset);
	virtual Quaternionf getRotationOffset();

	BoundBox getLocalBound() const;
	BoundBox getCustomSpaceBound(const Matrix4f& localToCustom) const;

	virtual PhysicalLayer getLayer() const;
	virtual void setLayer(const PhysicalLayer& layer);

	virtual void apply();
};

class PhysicalBody : public PhysicalBase
{
public:
	Serialize(PhysicalBody, PhysicalBase);

	enum BodyType {
		NONE, RIGID, SOFT
	} bodyType = NONE;
	enum LockFlag {
		None = 0, LinearX = 1 << 0, LinearY = 1 << 1, LinearZ = 1 << 2,
		AngularX = 1 << 3, AngularY = 1 << 4, AngularZ = 1 << 5,
	};
	Enum<LockFlag> lockFlags = None;
	PhysicalWorld* physicalWorld = NULL;

	vector<PhysicalCollider*> colliders;

	::Transform& targetTransform;
	PhysicalMaterial material;
	PxMaterial* rawMaterial = NULL;
#ifdef PHYSICS_USE_BULLET
	PhysicalBodyMotionState motionState;
#endif // !PHYSICS_USE_BULLET

#ifdef PHYSICS_USE_PHYSX
	
#endif // !PHYSICS_USE_PHYSX

	Delegate<void(PhysicalBody*, PhysicalWorld*)> onStepSimulation;

	PhysicalBody(::Transform& targetTransform, const PhysicalMaterial& material);
	virtual ~PhysicalBody();

	virtual void initBody();
	virtual PhysicalCollider* addCollider(Shape* shape, ShapeComplexType shapeComplexType = SIMPLE);
	virtual void updateObjectTransform();
	virtual void setWorldTransform(const Vector3f& position, const Quaternionf& rotation);
	virtual void handleCollision(const ContactInfo& info);
	virtual void addToWorld(PhysicalWorld& physicalWorld);
	virtual void removeFromWorld();

	virtual BoundBox getLocalBound() const;
	virtual BoundBox getWorldBound() const;
	virtual BoundBox getCustomSpaceBound(const Matrix4f& localToCustom) const;

	virtual int getColliderCount() const;
	virtual PhysicalCollider* getCollider(int index = 0);

	virtual CollisionObject* getCollisionObject() const;
	virtual void* getSoftObject() const;

	virtual Enum<LockFlag> getLockFlags() const;
	virtual void setLockFlags(Enum<LockFlag> flags);

	static Serializable* instantiate(const SerializationInfo& from);
};

#endif // !_PHYSICALBODY_H_