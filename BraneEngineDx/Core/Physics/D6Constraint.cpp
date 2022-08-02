#include "D6Constraint.h"
#include "PhysicalWorld.h"

D6Constraint::D6Constraint(RigidBody& rigidBody1, RigidBody& rigidBody2)
	: PhysicalConstraint(rigidBody1, rigidBody2)
{
}

D6Constraint::~D6Constraint()
{
}

void D6Constraint::addToWorld(PhysicalWorld& physicalWorld)
{
#ifdef PHYSICS_USE_PHYSX
	PxRigidActor* rigidActor1 = (PxRigidActor*)rigidBody1->getCollisionObject();
	PxRigidActor* rigidActor2 = (PxRigidActor*)rigidBody2->getCollisionObject();
	if (rigidActor1 == NULL || rigidActor2 == NULL)
		return;
	Vector3f scale1 = rigidBody1->targetTransform.getScale(WORLD);
	Vector3f scale2 = rigidBody2->targetTransform.getScale(WORLD);
	PTransform frameTransform1 = this->frameTransform1;
	PTransform frameTransform2 = this->frameTransform2;
	frameTransform1.p.x *= scale1.x();
	frameTransform1.p.y *= scale1.y();
	frameTransform1.p.z *= scale1.z();

	frameTransform2.p.x *= scale2.x();
	frameTransform2.p.y *= scale2.y();
	frameTransform2.p.z *= scale2.z();
	rawConstraint = PxD6JointCreate(*physicalWorld.gPhysicsSDK,
		rigidActor1, frameTransform1,
		rigidActor2, frameTransform2);
	rawConstraint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
	_configure();
#endif // !PHYSICS_USE_PHYSX
	this->physicalWorld = &physicalWorld;
}

void D6Constraint::configure(D6Config config)
{
	d6Config = config;
	_configure();
}

D6Constraint::D6Config D6Constraint::getConfigure()
{
	return d6Config;
}

void D6Constraint::_configure()
{
	if (rawConstraint == NULL)
		return;
	Vector3f scale1 = rigidBody1->targetTransform.getScale(WORLD);
	Vector3f scale2 = rigidBody2->targetTransform.getScale(WORLD);
	Vector3f scale = (scale1 + scale2) * 0.5f;
	PxD6Joint* joint = (PxD6Joint*)rawConstraint;
	joint->setMotion(PxD6Axis::eX, (PxD6Motion::Enum)d6Config.xMotion);
	joint->setMotion(PxD6Axis::eY, (PxD6Motion::Enum)d6Config.yMotion);
	joint->setMotion(PxD6Axis::eZ, (PxD6Motion::Enum)d6Config.zMotion);
	joint->setMotion(PxD6Axis::eTWIST, (PxD6Motion::Enum)d6Config.twistMotion);
	joint->setMotion(PxD6Axis::eSWING1, (PxD6Motion::Enum)d6Config.swing1Motion);
	joint->setMotion(PxD6Axis::eSWING2, (PxD6Motion::Enum)d6Config.swing2Motion);
	joint->setLinearLimit(PxD6Axis::eX,
		PxJointLinearLimitPair(d6Config.positionLimit.min.x() * scale.x(), d6Config.positionLimit.max.x() * scale.x(),
			PxSpring(d6Config.positionSpring.x() * scale.x(), d6Config.positionDamping.x() * scale.x())));
	joint->setLinearLimit(PxD6Axis::eY,
		PxJointLinearLimitPair(d6Config.positionLimit.min.y() * scale.y(), d6Config.positionLimit.max.y() * scale.y(),
			PxSpring(d6Config.positionSpring.y() * scale.y(), d6Config.positionDamping.y() * scale.y())));
	joint->setLinearLimit(PxD6Axis::eZ,
		PxJointLinearLimitPair(d6Config.positionLimit.min.z() * scale.z(), d6Config.positionLimit.max.z() * scale.z(),
			PxSpring(d6Config.positionSpring.z() * scale.z(), d6Config.positionDamping.z() * scale.z())));
	joint->setTwistLimit(PxJointAngularLimitPair(
		d6Config.rotationLimit.min.x(), d6Config.rotationLimit.max.x(),
		PxSpring(d6Config.rotationSpring.x(), d6Config.rotationDamping.x())));
	joint->setPyramidSwingLimit(PxJointLimitPyramid(
		d6Config.rotationLimit.min.y(), d6Config.rotationLimit.max.y(),
		d6Config.rotationLimit.min.z(), d6Config.rotationLimit.max.z()));
	joint->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, d6Config.enableCollision);
}
