#include "PhysicalConstraint.h"
#include "PhysicalWorld.h"

PhysicalConstraint::PhysicalConstraint(RigidBody & rigidBody1, RigidBody & rigidBody2)
	: rigidBody1(&rigidBody1), rigidBody2(&rigidBody2)
{
}

PhysicalConstraint::~PhysicalConstraint()
{
	if (rawConstraint != NULL)
		rawConstraint->release();
}

void PhysicalConstraint::addToWorld(PhysicalWorld& physicalWorld)
{

}

void PhysicalConstraint::removeFromWorld()
{
	if (rawConstraint != NULL)
		rawConstraint->release();
	rawConstraint = NULL;
}

PConstraint* PhysicalConstraint::getPConstraint() const
{
	return rawConstraint;
}
