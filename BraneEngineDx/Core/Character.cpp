#include "Character.h"
#include "Asset.h"

SerializeInstance(Character);

Character::Character(Capsule capsule, string name)
	: Actor(name), physicalController(*this, capsule)
{
}

void Character::tick(float deltaTime)
{
	Actor::tick(deltaTime);
	physicalController.update(deltaTime);
}

void Character::afterTick()
{
	Actor::afterTick();
	physicalController.updateObjectTransform();
}

void Character::setupPhysics(PhysicalWorld & physicalWorld)
{
	Actor::setupPhysics(physicalWorld);
	if (physicalController.physicalWorld == NULL)
		physicalController.addToWorld(physicalWorld);
}

void Character::releasePhysics(PhysicalWorld & physicalWorld)
{
	Actor::releasePhysics(physicalWorld);
	physicalController.removeFromWorld();
}

bool Character::isFly()
{
	return physicalController.isFly();
}

void Character::move(const Vector3f & v)
{
	physicalController.move(v);
}

void Character::jump(float impulse)
{
	physicalController.jump(impulse);
}

Serializable * Character::instantiate(const SerializationInfo & from)
{
	Capsule capsule;
	from.get("capsule", capsule);
	Character* character = new Character(capsule, from.name);
	ChildrenInstantiateObject(from, character);
	return character;
}

bool Character::deserialize(const SerializationInfo & from)
{
	return Actor::deserialize(from);
}

bool Character::serialize(SerializationInfo & to)
{
	if (!Actor::serialize(to))
		return false;
	to.type = "Character";
	to.set("capsule", physicalController.capsule);
	return true;
}
