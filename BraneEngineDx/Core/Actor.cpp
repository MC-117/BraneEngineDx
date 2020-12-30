#include "Actor.h"
#include "Asset.h"

SerializeInstance(Actor);

Actor::Actor(string name) : Transform(name)
{
}

Actor::~Actor()
{
}

void Actor::begin()
{
	Transform::begin();
}

void Actor::tick(float deltaTime)
{
	Transform::tick(deltaTime);
}

void Actor::afterTick()
{
	Transform::afterTick();
}

void Actor::end()
{
	::Transform::end();
}

void Actor::prerender()
{
}

void Actor::setHidden(bool value)
{
}

bool Actor::isHidden()
{
	return false;
}

Serializable * Actor::instantiate(const SerializationInfo & from)
{
	return new Actor(from.name);
}

bool Actor::deserialize(const SerializationInfo & from)
{
	if (!::Transform::deserialize(from))
		return false;
	return true;
}

bool Actor::serialize(SerializationInfo & to)
{
	if (!::Transform::serialize(to))
		return false;
	to.type = "Actor";
	return true;
}
