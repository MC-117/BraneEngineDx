#pragma once
#include "../Base.h"

class PhysicalWorld;

class ENGINE_API PhysicalBase : public Base
{
public:
	Serialize(PhysicalBase, Base);

	virtual void addToWorld(PhysicalWorld& physicalWorld) = 0;
	virtual void removeFromWorld() = 0;

	static Serializable* instantiate(const SerializationInfo& from);
};