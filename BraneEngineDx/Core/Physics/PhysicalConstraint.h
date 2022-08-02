#pragma once
#ifndef _CONSTRAIN_H_
#define _CONSTRAIN_H_

#include "../Transform.h"
#include "PhysicalMaterial.h"
#include "RigidBody.h"

class PhysicalConstraint
{
public:
	PhysicalWorld* physicalWorld = NULL;
	RigidBody* rigidBody1 = NULL;
	RigidBody* rigidBody2 = NULL;
	PTransform frameTransform1 = PTransform(PxIdentity);
	PTransform frameTransform2 = PTransform(PxIdentity);

	PhysicalConstraint(RigidBody& rigidBody1, RigidBody& rigidBody2);
	virtual ~PhysicalConstraint();

	virtual void addToWorld(PhysicalWorld& physicalWorld);
	virtual void removeFromWorld();

	virtual PConstraint* getPConstraint() const;
protected:
	PConstraint* rawConstraint = NULL;
};

#endif _CONSTRAIN_H_