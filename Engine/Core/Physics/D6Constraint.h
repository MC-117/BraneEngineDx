#pragma once
#include "PhysicalConstraint.h"

class D6Constraint : public PhysicalConstraint
{
public:
	Serialize(D6Constraint, PhysicalConstraint);

	enum MotionType
	{
		Locked, Limited, Free
	};

	struct Limit3
	{
		Vector3f min;
		Vector3f max;
	};

	struct D6Config
	{
		bool enableCollision;
		MotionType xMotion;
		MotionType yMotion;
		MotionType zMotion;
		MotionType twistMotion;
		MotionType swing1Motion;
		MotionType swing2Motion;
		Limit3 positionLimit;
		Limit3 rotationLimit;
		Vector3f positionSpring;
		Vector3f positionDamping;
		Vector3f rotationSpring;
		Vector3f rotationDamping;

		D6Config();
	};

	D6Constraint(RigidBody& rigidBody1, RigidBody& rigidBody2);
	virtual ~D6Constraint();

	virtual void addToWorld(PhysicalWorld& physicalWorld);
	void configure(D6Config config);
	D6Config getConfigure();

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	D6Config d6Config;
	void _configure();
};