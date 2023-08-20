#include "FixedConstraint.h"
#include "PhysicalWorld.h"

FixedConstraint::FixedConstraint(RigidBody& rigidBody1, RigidBody& rigidBody2)
	: PhysicalConstraint(rigidBody1, rigidBody2)
{
}

FixedConstraint::~FixedConstraint()
{
}

void FixedConstraint::addToWorld(PhysicalWorld& physicalWorld)
{
#ifdef PHYSICS_USE_PHYSX
	rawConstraint = PxFixedJointCreate(*physicalWorld.gPhysicsSDK,
		(PxRigidActor*)rigidBody1->getCollisionObject(), frameTransform1,
		(PxRigidActor*)rigidBody2->getCollisionObject(), frameTransform2);
	rawConstraint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
#endif // !PHYSICS_USE_PHYSX
	this->physicalWorld = &physicalWorld;
}