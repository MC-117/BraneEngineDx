#pragma once

#include "../Unit.h"

struct ExtentBox;
struct BoundBox
{
	Vector3f minPoint;
	Vector3f maxPoint;

	static BoundBox none;

	BoundBox() = default;
	BoundBox(const Vector3f& minPoint, const Vector3f& maxPoint);
	BoundBox(const ExtentBox& extentBox);

	bool isNone() const;

	Vector3f getCenter() const;
	Vector3f getExtent() const;
	Vector3f getSize() const;

	void encapsulate(const BoundBox& box);
	void encapsulate(const Vector3f& point);

	bool isInBox(const Vector3f& point) const;

	Vector4f getBoundSphere() const;

	bool operator==(const BoundBox& box) const;
	bool operator!=(const BoundBox& box) const;
};

struct ExtentBox
{
	Vector3f center;
	Vector3f extent;

	static ExtentBox none;

	ExtentBox() = default;
	ExtentBox(const Vector3f& center, const Vector3f& extent);
	ExtentBox(const BoundBox& boundBox);

	bool isNone() const;

	Vector3f getMinPoint() const;
	Vector3f getMaxPoint() const;

	bool isInBox(const Vector3f& point) const;

	Vector4f getBoundSphere() const;

	bool operator==(const ExtentBox& box) const;
	bool operator!=(const ExtentBox& box) const;
};