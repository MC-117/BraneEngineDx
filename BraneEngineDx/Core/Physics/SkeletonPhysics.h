#pragma once
#include "D6Constraint.h"

class SkeletonMeshActor;

class SkeletonPhysics
{
public:
	struct ShapeInfo
	{
		enum ShapeType {
			NoneShape, BoxShape, SphereShape, CapsuleShape
		};

		ShapeType type = ShapeType::NoneShape;
		Range<Vector3f> bound;

		void serialize(SerializationInfo& info);
		void deserialize(const SerializationInfo& info);
		void fromShape(Shape* shape);
		Shape* createShape() const;
	};

	struct RigidBodyInfo
	{
		string boneName;
		PhysicalLayer layer;
		Enum<PhysicalBody::LockFlag> lockFlags;
		PhysicalMaterial material;
		ShapeInfo shape;
		Vector3f positionOffset;
		Quaternionf rotationOffset = Quaternionf::Identity();

		void serialize(SerializationInfo& info);
		void deserialize(const SerializationInfo& info);
	};

	struct D6ConstraintInfo
	{
		string boneName;
		string targetBoneName;
		Vector3f positionOffset;
		Vector3f targetPositionOffset;
		Quaternionf rotationOffset = Quaternionf::Identity();
		Quaternionf targetRotationOffset = Quaternionf::Identity();
		D6Constraint::D6Config config;

		void serialize(SerializationInfo& info);
		void deserialize(const SerializationInfo& info);
	};

	struct SkeletonPhysicsInfo
	{
		vector<RigidBodyInfo> rigidbodies;
		vector<D6ConstraintInfo> constraints;

		void serialize(SerializationInfo& info);
		void deserialize(const SerializationInfo& info);
	};

	SkeletonMeshActor* actor = NULL;
	SkeletonPhysicsInfo info;
	vector<RigidBody*> rigidbodies;
	vector<D6Constraint*> constraints;

	SkeletonPhysics(SkeletonMeshActor* actor);

	SkeletonPhysicsInfo& getSkeletonPhysicsInfo();
	void applySkeletonPhysicsInfo();
};

