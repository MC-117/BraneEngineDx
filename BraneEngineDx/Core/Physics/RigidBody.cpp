#include "RigidBody.h"
#include "PhysicalWorld.h"

RigidBodyCollider::RigidBodyCollider(Shape* shape, ShapeComplexType shapeComplexType, Vector3f scale)
	: PhysicalCollider(shape, shapeComplexType, scale)
{
}

RigidBodyCollider::~RigidBodyCollider()
{
	if (collisionShape != NULL) {
		delete collisionShape;
		collisionShape = NULL;
	}
}

void RigidBodyCollider::setPositionOffset(const Vector3f& offset)
{
	if (positionOffset == offset)
		return;
	positionOffset = offset;
	apply();
}

void RigidBodyCollider::setRotationOffset(const Quaternionf& offset)
{
	if (rotationOffset == offset)
		return;
	rotationOffset = offset;
	apply();
}

void RigidBodyCollider::setLayer(const PhysicalLayer& layer)
{
	this->layer = layer;
	if (rawShape != NULL)
		rawShape->setSimulationFilterData(layer);
}

PxShape* RigidBodyCollider::getRawShape(PxPhysics& physics)
{
	if (rawShape == NULL) {
		collisionShape = NULL;
		switch (shapeComplexType)
		{
		case COMPLEX:
			collisionShape = shape->generateComplexCollisionShape(localScale);
			break;
		case SIMPLE:
			collisionShape = shape->generateCollisionShape(localScale);
			break;
		case NONESHAPE:
			break;
		default:
			break;
		}
		if (shapeComplexType != NONESHAPE && collisionShape == NULL) {
			collisionShape = shape->generateCollisionShape();
			shapeComplexType = SIMPLE;
		}
		rawShape = physics.createShape(*collisionShape, *rawMaterial, true);
		rawShape->userData = this;
	}
	return rawShape;
}

void RigidBodyCollider::apply()
{
	if (rawShape == NULL)
		return;
	rawShape->setLocalPose(getOffsetTransform());
	rawShape->setSimulationFilterData(layer);
}

PTransform RigidBodyCollider::getOffsetTransform() const
{
	return shapeComplexType == SIMPLE ?
		toPTransform((shape->getCenter() + positionOffset).cwiseProduct(localScale), rotationOffset)
		: toPTransform(positionOffset.cwiseProduct(localScale), rotationOffset);
}

RigidBody::RigidBody(::Transform& targetTransform, const PhysicalMaterial& material)
	: PhysicalBody::PhysicalBody(targetTransform, material)
#ifdef PHYSICS_USE_BULLET
	,CollisionRigidBody::btRigidBody(CollisionRigidBody::btRigidBodyConstructionInfo(material.mass, &motionState, collisionShape))
#endif
{
	bodyType = RIGID;
#ifdef PHYSICS_USE_BULLET
	PVec3 l;
	collisionShape->calculateLocalInertia(material.mass, l);
	setMassProps(material.mass, l);
	switch (material.physicalType)
	{
	case DYNAMIC:
		break;
	case STATIC:
		setCollisionFlags(getCollisionFlags() | CF_KINEMATIC_OBJECT);
		setActivationState(DISABLE_DEACTIVATION);
		break;
	case NOCOLLISIOIN:
		setCollisionFlags(getCollisionFlags() | CF_NO_CONTACT_RESPONSE);
		break;
	default:
		break;
	}
	updateInertiaTensor();
	setRestitution(0);
#endif
}

RigidBody::~RigidBody()
{
#ifdef PHYSICS_USE_PHYSX
	if (rawRigidActor != NULL) {
		rawRigidActor->release();
	}
#endif // PHYSICS_USE_PHYSX
}

void RigidBody::initBody()
{
#ifdef PHYSICS_USE_BULLET
	setWorldTransform(targetTransform.getWorldTransform() * motionState.m_centerOfMassOffset.inverse());
#endif
#ifdef PHYSICS_USE_PHYSX
	rawRigidActor->setGlobalPose(targetTransform.getWorldTransform());
#endif
}

PhysicalCollider* RigidBody::addCollider(Shape* shape, ShapeComplexType shapeComplexType)
{
	RigidBodyCollider* collider = new RigidBodyCollider(shape, shapeComplexType, targetTransform.getScale(WORLD));
	collider->body = this;
	collider->rawMaterial = rawMaterial;
	colliders.push_back(collider);

	if (rawRigidActor) {
		PxShape* shape = collider->getRawShape(*PhysicalWorld::gPhysicsSDK);
		if (shape)
		{
			bool status = rawRigidActor->attachShape(*shape);
			shape->release();
			shape->setFlag(PxShapeFlag::eVISUALIZATION, true);
			if (!status) {
				collider->rawShape = NULL;
			}
		}
		collider->apply();
	}
	return collider;
}

void RigidBody::updateObjectTransform()
{
#ifdef PHYSICS_USE_BULLET
	if (material.physicalType == PhysicalType::DYNAMIC) {
		targetTransform.apply(getWorldTransform());
	}
	else if (material.physicalType == PhysicalType::STATIC) {
		setWorldTransform(targetTransform.getWorldTransform());
	}
#endif
#ifdef PHYSICS_USE_PHYSX
	if (physicalWorld == NULL)
		return;
	if (material.physicalType == PhysicalType::DYNAMIC) {
		targetTransform.apply(rawRigidDynamic->getGlobalPose());
		/*PTransform ppose = rawRigidBody->getGlobalPose();
		Vector3f pos;
		bool updatePose = false;
		if ((int)lockFlags.intersect(LinearX | LinearY | LinearZ)) {
			Vector3f _pos = targetTransform.getPosition(WORLD);
			if (lockFlags.has(LinearX))
				pos.x() = _pos.x();
			if (lockFlags.has(LinearY))
				pos.y() = _pos.y();
			if (lockFlags.has(LinearZ))
				pos.z() = _pos.z();
			updatePose = true;
		}
		else
			pos = toVector3f(ppose.p);
		Quaternionf quat;
		if ((int)lockFlags.intersect(AngularX | AngularY | AngularZ)) {
			Vector3f eular = toQuaternionf(ppose.q).toRotationMatrix().eulerAngles();
			Vector3f _eular = targetTransform.getEulerAngle(WORLD);
			if (lockFlags.has(AngularX))
				eular.x() = _eular.x();
			if (lockFlags.has(AngularY))
				eular.y() = _eular.y();
			if (lockFlags.has(AngularZ))
				eular.z() = _eular.z();
			quat.setFromEularAngles(eular);
			updatePose = true;
		}
		else
			quat = toQuaternionf(ppose.q);
		targetTransform.setPosition(pos, WORLD);
		targetTransform.setRotation(quat, WORLD);
		if (updatePose) {
			ppose.p = toPVec3(pos);
			ppose.q = toPQuat(quat);
			rawRigidBody->setGlobalPose(ppose);
		}*/
	}
	else if (material.physicalType == PhysicalType::STATIC) {
		rawRigidDynamic->setKinematicTarget(targetTransform.getWorldTransform());
	}
	else if (material.physicalType == PhysicalType::NOCOLLISIOIN)
		rawRigidDynamic->setGlobalPose(targetTransform.getWorldTransform());
#endif
}

void RigidBody::addToWorld(PhysicalWorld & physicalWorld)
{
	PhysicalBody::addToWorld(physicalWorld);
#ifdef PHYSICS_USE_BULLET
	switch (bodyType)
	{
	case PhysicalBody::RIGID:
		physicalWorld.physicsScene->addRigidBody((CollisionRigidBody*)(getCollisionObject()));
		break;
	default:
		break;
	}
#endif // !PHYSICS_USE_BULLET
#ifdef PHYSICS_USE_PHYSX
	Vector3f localSca = targetTransform.getScale(WORLD);
	float massScale = localSca.x() * localSca.y() * localSca.z();
	if (material.physicalType == TERRAIN) {
		rawRigidActor = PhysicalWorld::gPhysicsSDK->createRigidStatic(targetTransform.getWorldTransform());
		rawRigidStatic = (PxRigidStatic*)rawRigidActor;
		rawRigidDynamic = NULL;
	}
	else {
		rawRigidActor = PhysicalWorld::gPhysicsSDK->createRigidDynamic(targetTransform.getWorldTransform());
		rawRigidDynamic = (PxRigidDynamic*)rawRigidActor;
		rawRigidStatic = NULL;
		rawRigidDynamic->setMass(material.mass * massScale);
		rawRigidDynamic->setAngularDamping(material.angularDamping);
		rawRigidDynamic->setLinearDamping(material.linearDamping);
		rawRigidDynamic->setRigidDynamicLockFlags((PxRigidDynamicLockFlag::Enum)(int)lockFlags);
		switch (material.physicalType)
		{
		case DYNAMIC:
			break;
		case STATIC:
			rawRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
			break;
		case NOCOLLISIOIN:
			rawRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, false);
			break;
		default:
			break;
		}
	}
	for (int i = 0; i < colliders.size(); i++) {
		RigidBodyCollider* collider = (RigidBodyCollider*)colliders[i];
		PxShape* shape = collider->getRawShape(*PhysicalWorld::gPhysicsSDK);
		if (shape)
		{
			bool status = rawRigidActor->attachShape(*shape);
			shape->release();
			shape->setFlag(PxShapeFlag::eVISUALIZATION, true);
			if (!status) {
				collider->rawShape = NULL;
			}
		}
		collider->localScale = localSca;
		collider->apply();
	}
	if (rawRigidDynamic)
		PxRigidBodyExt::updateMassAndInertia(*rawRigidDynamic, 1);
	physicalWorld.physicsScene->addActor(*getCollisionObject());
#endif // !PHYSICS_USE_PHYSX
	this->physicalWorld = &physicalWorld;
	initBody();
}

void RigidBody::removeFromWorld()
{
	PhysicalBody::removeFromWorld();
	if (physicalWorld == NULL)
		return;
#ifdef PHYSICS_USE_BULLET
	switch (bodyType)
	{
	case PhysicalBody::RIGID:
		physicalWorld->physicsScene->removeRigidBody((CollisionRigidBody*)(getCollisionObject()));
		break;
	default:
		break;
	}
#endif // !PHYSICS_USE_BULLET
#ifdef PHYSICS_USE_PHYSX
	if (getCollisionObject() != NULL)
		physicalWorld->physicsScene->removeActor(*getCollisionObject());
#endif // !PHYSICS_USE_PHYSX
	physicalWorld = NULL;
}

void RigidBody::setMass(float mass)
{
	material.mass = mass;
	if (rawRigidDynamic) {
		Vector3f localSca = targetTransform.getScale(WORLD);
		float massScale = localSca.x() * localSca.y() * localSca.z();
		rawRigidDynamic->setMass(mass * massScale);
	}
}

void RigidBody::setLockFlags(Enum<LockFlag> flags)
{
	PhysicalBody::setLockFlags(flags);
	if (rawRigidDynamic) {
		rawRigidDynamic->setRigidDynamicLockFlags((PxRigidDynamicLockFlag::Enum)(int)flags);
	}
}

CollisionObject * RigidBody::getCollisionObject() const
{
#ifdef PHYSICS_USE_BULLET
	return (CollisionObject*)this;
#endif
#ifdef PHYSICS_USE_PHYSX
	return rawRigidActor;
#endif
}

void RigidBody::addForce(const Vector3f& force, bool autoWake)
{
	if (rawRigidDynamic) {
		rawRigidDynamic->addForce(toPVec3(force), PxForceMode::eFORCE, autoWake);
	}
}

void RigidBody::addImpulse(const Vector3f& impulse, bool autoWake)
{
	if (rawRigidDynamic) {
		rawRigidDynamic->addForce(toPVec3(impulse), PxForceMode::eIMPULSE, autoWake);
	}
}

void RigidBody::addAcceleration(const Vector3f& acceleration, bool autoWake)
{
	if (rawRigidDynamic) {
		rawRigidDynamic->addForce(toPVec3(acceleration), PxForceMode::eACCELERATION, autoWake);
	}
}

void RigidBody::addVelocity(const Vector3f& velocity, bool autoWake)
{
	if (rawRigidDynamic) {
		rawRigidDynamic->addForce(toPVec3(velocity), PxForceMode::eVELOCITY_CHANGE, autoWake);
	}
}

void RigidBody::addForceAtLocation(const Vector3f& force, const Vector3f& location, bool autoWake)
{
	if (rawRigidDynamic) {
		PxRigidBodyExt::addForceAtPos(*rawRigidDynamic, toPVec3(force),
			toPVec3(location), PxForceMode::eFORCE, autoWake);
	}
}

void RigidBody::addImpulseAtLocation(const Vector3f& impulse, const Vector3f& location, bool autoWake)
{
	if (rawRigidDynamic) {
		PxRigidBodyExt::addForceAtPos(*rawRigidDynamic, toPVec3(impulse),
			toPVec3(location), PxForceMode::eIMPULSE, autoWake);
	}
}
