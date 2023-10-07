#include "VehicleActor.h"
#include "../Core/Engine.h"
#include "../Core/Console.h"

WheelActor::WheelActor(Mesh& mesh, Material& material, string name)
	: MeshActor(mesh, material, name)
{
	desc.radius = mesh.getHeight() / 2;
	desc.width = mesh.getWidth();
}

void WheelActor::setTorque(float T, float brake, const Vector3f& direction, float applyMess)
{
	float brakeF;
	if (state.w.y() == 0)
		brake = 0;
	else if (state.w.y() > 0)
		brake = -brake;
	state.F = getForward(WORLD) * (T / desc.radius) + brake;
	float tforceAtForword = -state.F.dot(direction);
	this->applyMess = applyMess;
	float mass = desc.mass + applyMess;
	float maxFriction = desc.friction * mass;
	if (isOnTheGround() && tforceAtForword > maxFriction)
		state.F += direction * (tforceAtForword - maxFriction);
}

void WheelActor::setSteer(float v)
{
	steer = v;
}

void WheelActor::update(float distance, float steerAngle)
{
	setPosition(desc.offset);
	rotAngle += distance / desc.radius * 2;
	setRotation(Quaternionf::FromAngleAxis(desc.steerAngle == 0 ? 0 : steerAngle, Vector3f::UnitZ()) *
		Quaternionf::FromAngleAxis(rotAngle, Vector3f::UnitY()));
}

Vector3f WheelActor::getForce()
{
	return state.F;
}

bool WheelActor::isOnTheGround() const
{
	return true;
}

VehicleActor::VehicleActor(Mesh& mesh, Material& material, string name)
	: MeshActor(mesh, material, name), TPCamera("VehicleTPCam"), FPCamera("VehicleFPCam")
{
	addChild(TPCamera);
	addChild(FPCamera);
}

WheelActor& VehicleActor::addWheel(const WheelDesc& wheelDesc, Mesh& mesh, Material& mat)
{
	WheelActor& wheel = *new WheelActor(mesh, mat, "wheel" + to_string(wheels.size()));
	wheel.desc = wheelDesc;
	addChild(wheel);
	wheels.push_back(&wheel);
	return wheel;
}

void VehicleActor::setSteer(float v)
{
	targetSteer = v;
}

void VehicleActor::setAccelerator(float v)
{
	force = v * desc.maxForce * (desc.maxVelocity - velocity) / desc.maxVelocity;
}

void VehicleActor::setBrake(float v)
{
	brake = v * desc.maxBrake;
}

float _lerp(float current, float target, float step)
{
	float diff = target - current;
	float len = abs(diff);
	if (len <= step)
		return target;
	return current + diff / len * step;
}

void VehicleActor::update(float deltaTime)
{
	currentSteer = _lerp(currentSteer, targetSteer, deltaTime * steerSpeed);
	state.F = Vector3f::Zero();
	state.M = Vector3f::Zero();

	float a = (force - brake) / desc.mass - desc.dumpFactor * (desc.maxVelocity - velocity) / desc.maxVelocity;
	float v0 = velocity;
	velocity += a * deltaTime;
	float s = 0;
	if (velocity >= 0)
		s = (v0 + 0.5 * a * deltaTime) * deltaTime;
	else
		velocity = 0;

	if (currentSteer == 0) {
		translate(s, 0, 0);
	}
	else {
		float d = abs(wheels[0]->desc.offset.x() - wheels[2]->desc.offset.x()) * 0.5;
		float r = d / sin(currentSteer * desc.steerAngle * 0.5f);
		float angle = s / r * 0.5;
		float sign = currentSteer / abs(currentSteer);
		float l = 2 * r * sin(angle);
		translate(l * cos(angle), l * sin(angle) * sign, 0);
		rotate(Quaternionf::FromAngleAxis(angle * 2, Vector3f::UnitZ()));
	}

	for (int i = 0; i < wheels.size(); i++) {
		wheels[i]->update(s, currentSteer * desc.steerAngle);
	}
}

bool VehicleActor::isControled() const
{
	World* world = Engine::getCurrentWorld();
	return world != NULL && (&world->getCurrentCamera() == &TPCamera ||
		&world->getCurrentCamera() == &FPCamera);
}

void VehicleActor::begin()
{
	MeshActor::begin();
	camDistance = TPCamera.distance;
}

void VehicleActor::tick(float deltaTime)
{
	World* world = Engine::getCurrentWorld();
	if (world != NULL) {
		bool ctrl = false;
		Input& input = Engine::getInput();
		if ((input.getCursorHidden() || input.getMouseButtonDown(MouseButtonEnum::Right))) {
			if (&world->getCurrentCamera() == &TPCamera) {
				ctrl = true;
				Unit2Di m = input.getMouseMove();
				TPCamera.rotate(0, 0, -m.x * deltaTime * 5, WORLD);
				TPCamera.rotate(0, m.y * deltaTime * 5, 0);
				camDistance += input.getMouseWheelValue();
				camDistance = clamp(camDistance, 20.f, 100.f);
				if (input.getKeyPress(VK_SPACE))
					world->switchCamera(FPCamera);
			}
			else if (&world->getCurrentCamera() == &FPCamera) {
				ctrl = true;
				Unit2Di m = input.getMouseMove();
				FPCamera.rotate(0, 0, -m.x * deltaTime * 5, WORLD);
				FPCamera.rotate(0, m.y * deltaTime * 5, 0);
				if (input.getKeyPress(VK_SPACE))
					world->switchCamera(TPCamera);
			}
		}
		if (ctrl) {
			setAccelerator(input.getKeyDown('W'));
			setBrake(input.getKeyDown('S'));
			setSteer(input.getKeyDown('A') - input.getKeyDown('D'));
		}
		else {
			setAccelerator(0);
			setBrake(0);
			setSteer(0);
		}
	}
	TPCamera.distance = _lerp(TPCamera.distance, camDistance, deltaTime * camSpeed);
	update(deltaTime);
}
