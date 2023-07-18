#pragma once
#ifndef _PHYSICALMATERIAL_H_
#define _PHYSICALMATERIAL_H_

#include "../Serialization.h"

enum PhysicalType {
	STATIC, DYNAMIC, NOCOLLISIOIN, TERRAIN
};

class PhysicalMaterial : public Serializable
{
public:
	Serialize(PhysicalMaterial,);
	float mass;
	PhysicalType physicalType;
	float angularDamping;
	float linearDamping;
	float friction;
	float restitution;

	PhysicalMaterial(float mass = 0, PhysicalType physicalType = STATIC);
	PhysicalMaterial(const PhysicalMaterial& mat);
	PhysicalMaterial(PhysicalMaterial&& mat);

	PhysicalMaterial& operator=(const PhysicalMaterial& mat);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

#endif // !_PHYSICALMATERIAL_H_
