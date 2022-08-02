#include "SphericalConstraint.h"
#include "PhysicalWorld.h"

SphericalConstraint::SphericalConstraint(RigidBody& rigidBody1, RigidBody& rigidBody2)
	: PhysicalConstraint(rigidBody1, rigidBody2)
{
}

SphericalConstraint::~SphericalConstraint()
{
}

void SphericalConstraint::addToWorld(PhysicalWorld& physicalWorld)
{
#ifdef PHYSICS_USE_PHYSX
	rawConstraint = PxSphericalJointCreate(*physicalWorld.gPhysicsSDK,
		(PxRigidActor*)rigidBody1->getCollisionObject(), frameTransform1,
		(PxRigidActor*)rigidBody2->getCollisionObject(), frameTransform2);
	rawConstraint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
#endif // !PHYSICS_USE_PHYSX
	this->physicalWorld = &physicalWorld;
}