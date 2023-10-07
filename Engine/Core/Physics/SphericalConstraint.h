#pragma once
#include "PhysicalConstraint.h"
class ENGINE_API SphericalConstraint : public PhysicalConstraint
{
public:
	SphericalConstraint(RigidBody& rigidBody1, RigidBody& rigidBody2);
	virtual ~SphericalConstraint();

	virtual void addToWorld(PhysicalWorld& physicalWorld);
};

