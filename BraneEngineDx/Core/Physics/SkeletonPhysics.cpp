#include "SkeletonPhysics.h"
#include "../SkeletonMeshActor.h"
#include "../Geometry.h"

void SkeletonPhysics::ShapeInfo::serialize(SerializationInfo& info)
{
	info.type = "ShapeInfo";
	info.set("type", (int)type);
	info.set("minPoint", SVector3f(bound.minVal));
	info.set("maxPoint", SVector3f(bound.maxVal));
}

void SkeletonPhysics::ShapeInfo::deserialize(const SerializationInfo& info)
{
	info.get("type", SEnum(&type));
	SVector3f minPoint;
	if (info.get("minPoint", minPoint))
		bound.minVal = minPoint;
	SVector3f maxPoint;
	if (info.get("maxPoint", maxPoint))
		bound.maxVal = maxPoint;
}

void SkeletonPhysics::ShapeInfo::fromShape(Shape* shape)
{
	if (shape == NULL)
		return;
	type = NoneShape;
	Box* box = dynamic_cast<Box*>(shape);
	if (box != NULL) {
		type = BoxShape;
	}
	else {
		Sphere* sphere = dynamic_cast<Sphere*>(shape);
		if (sphere != NULL) {
			type = SphereShape;
		}
		else {
			Capsule* capsule = dynamic_cast<Capsule*>(shape);
			if (capsule != NULL) {
				type = CapsuleShape;
			}
		}
	}
	bound = shape->bound;
}

Shape* SkeletonPhysics::ShapeInfo::createShape() const
{
	Shape* shape = NULL;
	switch (type)
	{
	case SkeletonPhysics::ShapeInfo::BoxShape:
		shape = new Box(bound.minVal, bound.maxVal);
		break;
	case SkeletonPhysics::ShapeInfo::SphereShape:
		shape = new Sphere(bound.minVal, bound.maxVal);
		break;
	case SkeletonPhysics::ShapeInfo::CapsuleShape:
		shape = new Capsule(bound.minVal, bound.maxVal);
		break;
	default:
		break;
	}
	return shape;
}

void SkeletonPhysics::RigidBodyInfo::serialize(SerializationInfo& info)
{
	info.type = "RigidBodyInfo";
	info.set("boneName", boneName);
	SerializationInfo* plinfo = info.add("physicalLayer");
	if (plinfo != NULL) {
		layer.serialize(*plinfo);
	}
	info.set("lockFlags", (int)lockFlags);
	info.set("physicalMaterial", material);
	SerializationInfo& shapInfo = *info.add("shape");
	shape.serialize(shapInfo);
	info.set("positionOffset", SVector3f(positionOffset));
	info.set("rotationOffset", SQuaternionf(rotationOffset));
}

void SkeletonPhysics::RigidBodyInfo::deserialize(const SerializationInfo& info)
{
	info.get("boneName", boneName);
	const SerializationInfo* plinfo = info.get("physicalLayer");
	if (plinfo != NULL) {
		layer.deserialize(*plinfo);
	}
	info.get("lockFlags", SEnum(&lockFlags));
	info.get("physicalMaterial", material);
	const SerializationInfo* shapInfo = info.get("shape");
	if (shapInfo != NULL)
		shape.deserialize(*shapInfo);
	SVector3f svec3;
	if (info.get("positionOffset", svec3))
		positionOffset = svec3;
	SQuaternionf squat;
	if (info.get("rotationOffset", squat))
		rotationOffset = squat;
}

void SkeletonPhysics::D6ConstraintInfo::serialize(SerializationInfo& info)
{
	info.type = "D6ConstraintInfo";
	info.set("boneName", boneName);
	info.set("targetBoneName", targetBoneName);

	info.set("positionOffset", SVector3f(positionOffset));
	info.set("targetPositionOffset", SVector3f(targetPositionOffset));
	info.set("rotationOffset", SQuaternionf(rotationOffset));
	info.set("targetRotationOffset", SQuaternionf(targetRotationOffset));

	info.set("enableCollision", config.enableCollision ? "true" : "false");

	info.set("xMotion", (int)config.xMotion);
	info.set("yMotion", (int)config.yMotion);
	info.set("zMotion", (int)config.zMotion);
	info.set("twistMotion", (int)config.twistMotion);
	info.set("swing1Motion", (int)config.swing1Motion);
	info.set("swing2Motion", (int)config.swing2Motion);
	info.set("positionLimitMax", SVector3f(config.positionLimit.max));
	info.set("positionLimitMin", SVector3f(config.positionLimit.min));
	info.set("positionSpring", SVector3f(config.positionSpring));
	info.set("positionDamping", SVector3f(config.positionDamping));
	info.set("rotationLimitMax", SVector3f(config.rotationLimit.max));
	info.set("rotationLimitMin", SVector3f(config.rotationLimit.min));
	info.set("rotationSpring", SVector3f(config.rotationSpring));
	info.set("rotationDamping", SVector3f(config.rotationDamping));
}

void SkeletonPhysics::D6ConstraintInfo::deserialize(const SerializationInfo& info)
{
	info.get("boneName", boneName);
	info.get("targetBoneName", targetBoneName);

	SVector3f svec3;
	if (info.get("positionOffset", svec3))
		positionOffset = svec3;
	if (info.get("targetPositionOffset", svec3))
		targetPositionOffset = svec3;
	SQuaternionf squat;
	if (info.get("rotationOffset", squat))
		rotationOffset = squat;
	if (info.get("targetRotationOffset", squat))
		targetRotationOffset = squat;

	string enableCollision;
	info.get("enableCollision", enableCollision);
	config.enableCollision = enableCollision == "true";

	info.get("xMotion", SEnum(&config.xMotion));
	info.get("yMotion", SEnum(&config.yMotion));
	info.get("zMotion", SEnum(&config.zMotion));
	info.get("twistMotion", SEnum(&config.twistMotion));
	info.get("swing1Motion", SEnum(&config.swing1Motion));
	info.get("swing2Motion", SEnum(&config.swing2Motion));
	if (info.get("positionLimitMax", svec3))
		config.positionLimit.max = svec3;
	if (info.get("positionLimitMin", svec3))
		config.positionLimit.min = svec3;
	if (info.get("positionSpring", svec3))
		config.positionSpring = svec3;
	if (info.get("positionDamping", svec3))
		config.positionDamping = svec3;
	if (info.get("rotationLimitMax", svec3))
		config.rotationLimit.max = svec3;
	if (info.get("rotationLimitMin", svec3))
		config.rotationLimit.min = svec3;
	if (info.get("rotationSpring", svec3))
		config.rotationSpring = svec3;
	if (info.get("rotationDamping", svec3))
		config.rotationDamping = svec3;
}

void SkeletonPhysics::SkeletonPhysicsInfo::serialize(SerializationInfo& info)
{
	info.type = "SkeletonPhysics";
	SerializationInfo& rigidbodiesInfo = *info.add("rigidbodies");
	rigidbodiesInfo.type = "Array";
	rigidbodiesInfo.arrayType = "RigidBodyInfo";
	for (auto b = rigidbodies.begin(), e = rigidbodies.end(); b != e; b++) {
		SerializationInfo& rinfo = *rigidbodiesInfo.push();
		b->serialize(rinfo);
	}

	SerializationInfo& constraintsInfo = *info.add("constraints");
	constraintsInfo.type = "Array";
	constraintsInfo.arrayType = "D6ConstraintInfo";
	for (auto b = constraints.begin(), e = constraints.end(); b != e; b++) {
		SerializationInfo& cinfo = *constraintsInfo.push();
		b->serialize(cinfo);
	}
}

void SkeletonPhysics::SkeletonPhysicsInfo::deserialize(const SerializationInfo& info)
{
	const SerializationInfo& rigidbodiesInfo = *info.get("rigidbodies");
	for (auto b = rigidbodiesInfo.sublists.begin(), e = rigidbodiesInfo.sublists.end(); b != e; b++) {
		rigidbodies.emplace_back();
		RigidBodyInfo& rbinfo = rigidbodies.back();
		rbinfo.deserialize(*b);
	}

	const SerializationInfo& constraintsInfo = *info.get("constraints");
	for (auto b = constraintsInfo.sublists.begin(), e = constraintsInfo.sublists.end(); b != e; b++) {
		constraints.emplace_back();
		D6ConstraintInfo& d6info = constraints.back();
		d6info.deserialize(*b);
	}
}

SkeletonPhysics::SkeletonPhysics(SkeletonMeshActor* actor) : actor(actor)
{
}

SkeletonPhysics::SkeletonPhysicsInfo& SkeletonPhysics::getSkeletonPhysicsInfo()
{
	if (actor == NULL)
		return info;

	info.rigidbodies.clear();
	info.constraints.clear();
	for (auto b = rigidbodies.begin(), e = rigidbodies.end(); b != e; b++) {
		for (int i = 0; i < (*b)->getColliderCount(); i++) {
			PhysicalCollider* collider = (*b)->getCollider(i);
			RigidBodyInfo& rbinfo = info.rigidbodies.emplace_back();
			rbinfo.boneName = (*b)->targetTransform.name;
			rbinfo.material = (*b)->material;
			rbinfo.shape.fromShape(collider->shape);
			rbinfo.layer = collider->getLayer();
			rbinfo.lockFlags = (*b)->getLockFlags();
			rbinfo.positionOffset = collider->getPositionOffset();
			rbinfo.rotationOffset = collider->getRotationOffset();
		}
	}

	for (auto b = constraints.begin(), e = constraints.end(); b != e; b++) {
		info.constraints.emplace_back();
		D6ConstraintInfo& cinfo = info.constraints.back();
		cinfo.boneName = (*b)->rigidBody2->targetTransform.name;
		cinfo.targetBoneName = (*b)->rigidBody1->targetTransform.name;
		cinfo.positionOffset = toVector3f((*b)->frameTransform1.p);
		cinfo.rotationOffset = toQuaternionf((*b)->frameTransform1.q);
		cinfo.targetPositionOffset = toVector3f((*b)->frameTransform2.p);
		cinfo.targetRotationOffset = toQuaternionf((*b)->frameTransform2.q);
		cinfo.config = (*b)->getConfigure();
	}
	return info;
}

void SkeletonPhysics::applySkeletonPhysicsInfo()
{
	if (actor == NULL)
		return;

	for (auto b = info.rigidbodies.begin(), e = info.rigidbodies.end(); b != e; b++) {
		Bone* bone = actor->getBone(b->boneName);
		if (bone != NULL) {
			Shape* shape = b->shape.createShape();
			if (bone->rigidBody == NULL) {
				bone->updataRigidBody(b->material);
				rigidbodies.push_back(bone->rigidBody);
			}
			bone->rigidBody->setLockFlags(b->lockFlags);
			PhysicalCollider* collider = bone->rigidBody->addCollider(shape, SIMPLE);
			collider->setPositionOffset(b->positionOffset);
			collider->setRotationOffset(b->rotationOffset);
			collider->setLayer(b->layer);
		}
	}

	for (auto b = info.constraints.begin(), e = info.constraints.end(); b != e; b++) {
		Bone* bone = actor->getBone(b->boneName);
		Bone* targetBone = actor->getBone(b->targetBoneName);
		if (bone != NULL && targetBone != NULL && bone->rigidBody != NULL && targetBone->rigidBody != NULL) {
			D6Constraint* constraint = new D6Constraint(*bone->rigidBody, *targetBone->rigidBody);
			constraint->frameTransform1 = toPTransform(b->positionOffset, b->rotationOffset);
			constraint->frameTransform2 = toPTransform(b->targetPositionOffset, b->targetRotationOffset);
			constraint->configure(b->config);
			bone->addConstraint(constraint);
			constraints.push_back(constraint);
		}
	}
}
