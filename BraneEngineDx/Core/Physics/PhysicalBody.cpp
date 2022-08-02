#include "PhysicalBody.h"
#include "PhysicalWorld.h"

#ifdef PHYSICS_USE_BULLET
PhysicalBodyMotionState::PhysicalBodyMotionState(::Transform & targetTransform, const PTransform& centerOfMassOffset)
	: btDefaultMotionState(targetTransform.getWorldTransform(), centerOfMassOffset), targetTransform(targetTransform)
{
}

void PhysicalBodyMotionState::getWorldTransform(PTransform & worldTrans) const
{
	worldTrans = targetTransform.getWorldTransform() * m_centerOfMassOffset.inverse();
}

void PhysicalBodyMotionState::setWorldTransform(const PTransform & worldTrans)
{
	btDefaultMotionState::setWorldTransform(worldTrans);
	targetTransform.apply(m_graphicsWorldTrans);
}
#endif // PHYSICS_USE_BULLET

PhysicalCollider::PhysicalCollider(Shape* shape, ShapeComplexType shapeComplexType, Vector3f scale)
	: shape(shape), shapeComplexType(shapeComplexType), localScale(scale)
{
}

PhysicalCollider::~PhysicalCollider()
{
	if (!isClassOf<Mesh>(shape)) {
		delete shape;
		shape = NULL;
	}
}

void PhysicalCollider::setPositionOffset(const Vector3f& offset)
{
	positionOffset = offset;
}

Vector3f PhysicalCollider::getPositionOffset()
{
	return positionOffset;
}

void PhysicalCollider::setRotationOffset(const Quaternionf& offset)
{
	rotationOffset = offset;
}

Quaternionf PhysicalCollider::getRotationOffset()
{
	return rotationOffset;
}

PhysicalLayer PhysicalCollider::getLayer() const
{
	return layer;
}

void PhysicalCollider::setLayer(const PhysicalLayer& layer)
{
	this->layer = layer;
}

void PhysicalCollider::apply()
{
}

PhysicalBody::PhysicalBody(::Transform& targetTransform, const PhysicalMaterial& material) :
#ifdef PHYSICS_USE_BULLET
	motionState(targetTransform, toPTransform(-shape->getCenter().cwiseProduct(targetTransform.scale))),
#endif // PHYSICS_USE_BULLET
	targetTransform(targetTransform), material(material)
{
	rawMaterial = PhysicalWorld::gPhysicsSDK->createMaterial(material.friction, material.friction, material.restitution);
}

PhysicalBody::~PhysicalBody()
{
	removeFromWorld();
	for (int i = 0; i < colliders.size(); i++) {
		delete colliders[i];
	}
	colliders.clear();
#ifdef PHYSICS_USE_BULLET
	if (collisionShape != NULL)
		delete collisionShape;
#endif
}

void PhysicalBody::initBody()
{
}

PhysicalCollider* PhysicalBody::addCollider(Shape* shape, ShapeComplexType shapeComplexType)
{
	return NULL;
}

void PhysicalBody::updateObjectTransform()
{
}

void PhysicalBody::handleCollision(const ContactInfo & info)
{
	targetTransform.collisionHappened(info);
}

void PhysicalBody::addToWorld(PhysicalWorld & physicalWorld)
{
	physicalWorld.dirty = true;
}

void PhysicalBody::removeFromWorld()
{
	if (physicalWorld != NULL)
		physicalWorld->dirty = true;
}

int PhysicalBody::getColliderCount() const
{
	return colliders.size();
}

PhysicalCollider* PhysicalBody::getCollider(int index)
{
	if (index < 0 || index >= colliders.size())
		return NULL;
	return colliders[index];
}

CollisionObject * PhysicalBody::getCollisionObject() const
{
	return nullptr;
}

void * PhysicalBody::getSoftObject() const
{
	return nullptr;
}

Enum<PhysicalBody::LockFlag> PhysicalBody::getLockFlags() const
{
	return lockFlags;
}

void PhysicalBody::setLockFlags(Enum<PhysicalBody::LockFlag> flags)
{
	lockFlags = flags;
}
