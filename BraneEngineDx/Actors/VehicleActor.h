#pragma once
#ifndef _VEHICLEACTOR_H_
#define _VEHICLEACTOR_H_

#include "../Core/MeshActor.h"
#include "../Core/PostProcess/PostProcessingCamera.h"

struct PhysicsState
{
	Vector3f F;
	Vector3f v;
	Vector3f M;
	Vector3f w = 0;
};

struct WheelDesc
{
	float radius = 20;
	float width = 10;
	float steerAngle = 0;
	float mass = 20;
	float friction = 0.5;
	Vector3f offset;
};

class WheelActor : public MeshActor
{
public:
	WheelDesc desc;

	WheelActor(Mesh& mesh, Material& material, string name = "WheelActor");

	void setTorque(float T, float brake, const Vector3f& direction, float applyMess);
	void setSteer(float v);

	void update(float distance, float steerAngle);

	Vector3f getForce();
	bool isOnTheGround() const;
//protected:
	PhysicsState state;
	float applyMess = 0;
	float torque = 0;
	float steer = 0;
	float force = 0;
	float rotAngle = 0;
};

struct VehicleDesc
{
	float maxVelocity = 100;
	float maxForce = 5000;
	float maxBrake = 10000;
	float mass = 1000;
	float steerAngle = 30 / 180.f * PI;
	float dumpFactor = 3;
};

class VehicleActor : public MeshActor
{
public:
	VehicleDesc desc;
	PostProcessingCamera TPCamera;
	PostProcessingCamera FPCamera;
	vector<WheelActor*> wheels;

	VehicleActor(Mesh& mesh, Material& material, string name = "VehicleActor");
	virtual ~VehicleActor();

	WheelActor& addWheel(const WheelDesc& wheelDesc, Mesh& mesh, Material& mat);

	void setSteer(float v);
	void setAccelerator(float v);
	void setBrake(float v);

	void update(float deltaTime);

	virtual void begin();
	virtual void tick(float deltaTime);
//protected:
	PhysicsState state;
	float targetSteer = 0;
	float currentSteer = 0;
	float steerSpeed = 10;
	float camSpeed = 20;
	float camDistance = 30;
	float force = 0;
	float brake = 0;
	float velocity = 0;
};

#endif // !_VEHICLEACTOR_H_
