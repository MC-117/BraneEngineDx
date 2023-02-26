#pragma once

#include "../Unit.h"

struct MeshTransformData
{
	Matrix4f localToWorld;
	Vector3f worldScale;
	float boundScale;
	Vector3f localCenter;
	float localRadius;
	Vector3f localExtent;
	unsigned int flag;

	MeshTransformData();

	bool operator==(const MeshTransformData& other) const;
	bool operator!=(const MeshTransformData& other) const;
};