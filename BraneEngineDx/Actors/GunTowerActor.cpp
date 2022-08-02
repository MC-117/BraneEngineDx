#include "GunTowerActor.h"
#include "../Core/Console.h"
#include "../Core/Engine.h"

GunTowerActor::GunTowerActor(SkeletonMesh& mesh, Material& material, string name)
	: SkeletonMeshActor(mesh, material, name), particleSystem("GunFirePS")
{
}

void GunTowerActor::setTargetDirection(const Vector3f& direction)
{
	targetDirection = direction;
}

void GunTowerActor::setYawBone(const string& boneName)
{
	yawBone = getBone(boneName);
}

void GunTowerActor::setPitchBone(const string& boneName)
{
	pitchBone = getBone(boneName);
	if (pitchBone != NULL)
		pitchBone->addChild(particleSystem);
}

void GunTowerActor::setRollBone(const string& boneName)
{
	rollBone = getBone(boneName);
}

void GunTowerActor::fire()
{
	targetGunSpeed = maxGunSpeed;
}

void GunTowerActor::unfire()
{
	targetGunSpeed = 0;
}

float lerp(float current, float target, float step)
{
	float diff = target - current;
	float len = abs(diff);
	if (len <= step)
		return target;
	return current + diff / len * step;
}

Vector3f lerp(const Vector3f& current, const Vector3f& target, float rate)
{
	return current * (1 - rate) + target;
}

float sign(float v)
{
	return v / abs(v);
}

void GunTowerActor::update(float deltaTime)
{
	Vector3f wFor = getForward(WORLD);
	Vector3f wLef = getRightward(WORLD);
	Vector3f wUp = getUpward(WORLD);
	currentDirection = lerp(currentDirection, targetDirection, deltaTime * directionStep).normalized();
	Vector2f xy = Vector2f(currentDirection.dot(wFor), currentDirection.dot(wLef)).normalized();
	float yaw = sign(xy.y()) * acos(xy.x()) * (180 / PI);
	float pitch = acos(currentDirection.dot(wUp)) * (180 / PI) - 90;

	currentGunSpeed = lerp(currentGunSpeed, targetGunSpeed, deltaTime * gunSpeedStep);

	if (yawBone != NULL)
		yawBone->setRotation(0, 0, yaw);
	if (pitchBone != NULL)
		pitchBone->setRotation(0, pitch, 0);
	if (rollBone != NULL)
		rollBone->rotate(currentGunSpeed, 0, 0);

	if (!particleSystem.particleRender.particleEmtters.empty()) {
		ParticleEmitter& emt = particleSystem.particleRender.particleEmtters[0];
		emt.emitCountSec = currentGunSpeed;
		float bulletSpeed = 1000;
		emt.initVelocity.maxVal = currentDirection * bulletSpeed;
		emt.initVelocity.minVal = emt.initVelocity.maxVal;
		emt.initScale.maxVal = { 0.1, bulletSpeed / 100, 0 };
		emt.initScale.minVal = emt.initScale.maxVal;
		if (!emt.isActivate())
			emt.activate();
	}
}

void GunTowerActor::tick(float deltaTime)
{
	World* world = Engine::getCurrentWorld();
	if (world != NULL) {
		targetDirection = world->getCurrentCamera().getForward(WORLD);
		Input& input = Engine::input;
		if ((input.getCursorHidden() || input.getMouseButtonDown(MouseButtonEnum::Right))) {
			if (input.getMouseButtonPress(MouseButtonEnum::Left))
				fire();
			if (input.getMouseButtonRelease(MouseButtonEnum::Left))
				unfire();
		}
	}
	update(deltaTime);
}
