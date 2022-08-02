#pragma once
#include "PhysicalConstraint.h"


class D6Constraint : public PhysicalConstraint
{
public:
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
	};

	D6Constraint(RigidBody& rigidBody1, RigidBody& rigidBody2);
	virtual ~D6Constraint();

	virtual void addToWorld(PhysicalWorld& physicalWorld);
	void configure(D6Config config);
	D6Config getConfigure();

protected:
	D6Config d6Config = {
		false,
		MotionType::Locked,
		MotionType::Locked,
		MotionType::Locked,
		MotionType::Free,
		MotionType::Free,
		MotionType::Free,
		Limit3 {
			Vector3f(-0.1f, -0.1f, -0.1f),
			Vector3f(0.1f,0.1f, 0.1f)
		},
		Limit3 {
			Vector3f(-PI * 0.25f, -PI * 0.25f, -PI * 0.25f),
			Vector3f(PI * 0.25f, PI * 0.25f, PI * 0.25f)
		},
		Vector3f(0.1f,0.1f, 0.1f),
		Vector3f(0.1f,0.1f, 0.1f),
		Vector3f(0.1f,0.1f, 0.1f),
		Vector3f(0.1f,0.1f, 0.1f)
	};
	void _configure();
};