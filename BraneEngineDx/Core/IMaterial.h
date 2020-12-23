#pragma once
#ifndef _IMATERIAL_H_
#define _IMATERIAL_H_

#include "Unit.h"

struct MaterialBaseInfo
{
	Matrix4f projectionViewMat;
	Matrix4f projectionMat;
	Matrix4f viewMat;
	Matrix4f lightSpaceMat;
	Vector3f cameraLoc;
	Vector3f cameraDir;
	Vector3f cameraUp;
	Vector3f cameraLeft;
	Vector2f viewSize;
	float zNear;
	float zFar;
	float fovy;
};

class IMaterial
{
public:
	virtual void processBaseData(const MaterialBaseInfo& info) const = 0;
	virtual void processBaseData();
	virtual void processScalarData();
	virtual void processCountData();
	virtual void processColorData();
	virtual void processTextureData();
	virtual void processImageData();
	virtual void processInstanceData();
};

#endif // !_IMATERIAL_H_
