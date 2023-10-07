#pragma once
#include "../Core/SkeletonMeshActor.h"
#include "../Core/Camera.h"
#include "../Core/ParticleSystem.h"
class ENGINE_API GunTowerActor : public SkeletonMeshActor
{
public:
	ParticleSystem particleSystem;

	GunTowerActor(SkeletonMesh& mesh, Material& material, string name = "GunTowerActor");

	void setTargetDirection(const Vector3f& direction);

	void setYawBone(const string& boneName);
	void setPitchBone(const string& boneName);
	void setRollBone(const string& boneName);

	void fire();
	void unfire();

	void update(float deltaTime);

	virtual void tick(float deltaTime);
protected:
	Bone* yawBone = NULL;
	Bone* pitchBone = NULL;
	Bone* rollBone = NULL;

	Vector3f targetDirection = Vector3f::UnitX();
	Vector3f currentDirection = Vector3f::UnitX();
	float directionStep = 0.05;
	float currentGunSpeed = 0;
	float targetGunSpeed = 0;
	float maxGunSpeed = 20;
	float gunSpeedStep = 10;
};

