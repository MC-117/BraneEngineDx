#pragma once

#include "../Unit.h"

enum MeshTransformFlags : unsigned int
{
	MeshTransform_None = 0,
	MeshTransform_Dynamic = 1 << 0,
	MeshTransform_NegScale = 1 << 1,
	MeshTransform_Bit_Mask = 0xFFFFFFFCU,
};

struct MeshTransformData
{
	Matrix4f localToWorld;
	Vector3f worldScale;
	float boundScale;
	Vector3f localCenter;
	float localRadius;
	Vector3f localExtent;
	unsigned int flag;
	unsigned int updateFrame;
	unsigned int objectIDHigh;
	unsigned int objectIDLow;
	unsigned int pad;

	MeshTransformData();

	bool operator==(const MeshTransformData& other) const;
	bool operator!=(const MeshTransformData& other) const;

	bool isDynamic() const;
	bool isNegativeScale() const;

	uint8_t getStencilValue() const;
	void setStencilValue(uint8_t stencil);
};