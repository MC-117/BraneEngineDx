#pragma once

#include "../Unit.h"

struct CameraData
{
	Matrix4f projectionViewMat;
	Matrix4f projectionMat;
	Matrix4f projectionMatInv;
	Matrix4f viewMat;
	Matrix4f viewMatInv;
	Matrix4f viewOriginMat;
	Matrix4f viewOriginMatInv;
	Vector3f cameraLoc;
	float zNear;
	Vector3f cameraDir;
	float zFar;
	Vector3f cameraUp;
	float fovy;
	Vector3f cameraLeft;
	float aspect;
	Vector2f viewSize;
	float distance;
	float user;
};

struct ScreenHitInfo
{
	Vector2u hitPosition;
	unsigned int hitDepth;
	unsigned int hitObjectIDHigh;
	Vector3f hitNormal;
	unsigned int hitObjectIDLow;
};