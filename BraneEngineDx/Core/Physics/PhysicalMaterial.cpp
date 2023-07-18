#include "PhysicalMaterial.h"

SerializeInstance(PhysicalMaterial);

PhysicalMaterial::PhysicalMaterial(float mass, PhysicalType physicalType)
	: mass(mass)
	, physicalType(physicalType)
	, angularDamping(0)
	, linearDamping(0)
	, friction(0.2)
	, restitution(0)
{
	if (physicalType == DYNAMIC && mass == 0)
		mass = 0.00001;
}

PhysicalMaterial::PhysicalMaterial(const PhysicalMaterial & mat)
{
	mass = mat.mass;
	physicalType = mat.physicalType;
	angularDamping = mat.angularDamping;
	linearDamping = mat.linearDamping;
	friction = mat.friction;
	restitution = mat.restitution;
}

PhysicalMaterial::PhysicalMaterial(PhysicalMaterial && mat)
{
	mass = mat.mass;
	physicalType = mat.physicalType;
	angularDamping = mat.angularDamping;
	linearDamping = mat.linearDamping;
	friction = mat.friction;
	restitution = mat.restitution;
}

PhysicalMaterial & PhysicalMaterial::operator=(const PhysicalMaterial & mat)
{
	mass = mat.mass;
	physicalType = mat.physicalType;
	angularDamping = mat.angularDamping;
	linearDamping = mat.linearDamping;
	friction = mat.friction;
	restitution = mat.restitution;
	return *this;
}

Serializable* PhysicalMaterial::instantiate(const SerializationInfo& from)
{
	return new PhysicalMaterial();
}

bool PhysicalMaterial::deserialize(const SerializationInfo& from)
{
	from.get("mass", mass);
	from.get("type", SEnum(&physicalType));
	from.get("angularDamping", angularDamping);
	from.get("linearDamping", linearDamping);
	from.get("friction", friction);
	from.get("restitution", restitution);
	return true;
}

bool PhysicalMaterial::serialize(SerializationInfo& to)
{
	serializeInit(this, to);
	to.set("mass", mass);
	to.set("type", (int)physicalType);
	to.set("angularDamping", angularDamping);
	to.set("linearDamping", linearDamping);
	to.set("friction", friction);
	to.set("restitution", restitution);
	return true;
}
