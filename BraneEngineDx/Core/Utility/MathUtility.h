#pragma once

#include "../MathLibrary.h"
#undef min
#undef max

bool IntersectAABB8Plane(const Vector3f& center, const Vector3f& extent, const Vector4f* planes);

namespace Math
{
	Vector2f abs(const Vector2f& a);
	Vector2f min(const Vector2f& a, const Vector2f& b);
	Vector2f max(const Vector2f& a, const Vector2f& b);
	float distance(const Vector2f& a, const Vector2f& b);

	Vector3f abs(const Vector3f& a);
	Vector3f min(const Vector3f& a, const Vector3f& b);
	Vector3f max(const Vector3f& a, const Vector3f& b);
	float distance(const Vector3f& a, const Vector3f& b);

	Vector4f abs(const Vector4f& a);
	Vector4f min(const Vector4f& a, const Vector4f& b);
	Vector4f max(const Vector4f& a, const Vector4f& b);
	float distance(const Vector4f& a, const Vector4f& b);

	Matrix4f getTransitionMatrix(const Vector3f& position);
	Matrix4f getRotationMatrix(const Quaternionf& rotation);
	Matrix4f getScaleMatrix(const Vector3f& scale);
	Matrix4f getTransformMatrix(const Vector3f& position, const Quaternionf& rotation, const Vector3f& scale);
}