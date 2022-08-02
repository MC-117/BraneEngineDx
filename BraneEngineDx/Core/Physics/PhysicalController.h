#pragma once
#ifndef _PHYSICALCONTROLLER_H_
#define _PHYSICALCONTROLLER_H_

#include "PhysicalBody.h"
#include "../Geometry.h"

class PhysicalController : public PhysicalBody, public PxUserControllerHitReport
{
public:
#ifdef PHYSICS_USE_PHYSX
	PxController* rawController = NULL;
#endif // !PHYSICS_USE_PHYSX
	Capsule capsule;

	Vector3f gravityScale = { 1, 1, 1 };
	Vector3f moveVector;
	Vector3f moveVelocity;
	Vector3f reservedMoveVelocity;
	Vector3f gravityVelocity;
	Vector3f orientation;
	bool hasCollision = false;

	PhysicalController(::Transform& targetTransform, Capsule capsule, const PhysicalMaterial& material = { 1000, DYNAMIC });
	
	virtual void initBody();
	virtual void updateObjectTransform();
	virtual void addToWorld(PhysicalWorld& physicalWorld);
	virtual void removeFromWorld();

	virtual bool isFly();
	virtual void move(const Vector3f& position);
	virtual void jump(float impulse);
	virtual void update(float deltaTime);

	virtual Vector3f getFootPosition() const;
	virtual Vector3f getPosition() const;
	virtual void setFootPosition(const Vector3f& position);
	virtual void setPosition(const Vector3f& position);

	virtual void onShapeHit(const PxControllerShapeHit& hit);
	virtual void onControllerHit(const PxControllersHit& hit);
	virtual void onObstacleHit(const PxControllerObstacleHit& hit);
protected:
	PxControllerState state;
};

#endif // !_PHYSICALCONTROLLER_H_
