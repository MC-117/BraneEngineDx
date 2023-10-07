#pragma once
#include "PhysicalConstraint.h"
class ENGINE_API FixedConstraint : public PhysicalConstraint
{
public:
	FixedConstraint(RigidBody& rigidBody1, RigidBody& rigidBody2);
	virtual ~FixedConstraint();

	virtual void addToWorld(PhysicalWorld& physicalWorld);
};

