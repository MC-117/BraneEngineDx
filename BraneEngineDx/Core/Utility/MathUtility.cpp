#include "MathUtility.h"

#define __MM_REPLICATE(V, I) _mm_shuffle_ps(V, V, _MM_SHUFFLE(I,I,I,I))

bool IntersectAABB8Plane(const Vector3f& center, const Vector3f& extent, const Vector4f* planes)
{
	using namespace DirectX;
	// this removes a lot of the branches as we know there's 8 planes
	// copied directly out of ConvexVolume.cpp
	const XMVECTOR Origin = XMLoadFloat3((XMFLOAT3*)center.data());
	const XMVECTOR Extent = XMLoadFloat3((XMFLOAT3*)extent.data());

	XMMATRIX Mat_0 = XMLoadFloat4x4((XMFLOAT4X4*)planes[0].data());
	Mat_0 = XMMatrixTranspose(Mat_0);

	XMMATRIX Mat_1 = XMLoadFloat4x4((XMFLOAT4X4*)planes[4].data());
	Mat_1 = XMMatrixTranspose(Mat_1);

	const XMVECTOR& PlanesX_0 = Mat_0.r[0];
	const XMVECTOR& PlanesY_0 = Mat_0.r[1];
	const XMVECTOR& PlanesZ_0 = Mat_0.r[2];
	const XMVECTOR& PlanesW_0 = Mat_0.r[3];

	const XMVECTOR& PlanesX_1 = Mat_1.r[0];
	const XMVECTOR& PlanesY_1 = Mat_1.r[1];
	const XMVECTOR& PlanesZ_1 = Mat_1.r[2];
	const XMVECTOR& PlanesW_1 = Mat_1.r[3];

	// Splat origin into 3 vectors
	XMVECTOR OrigX = __MM_REPLICATE(Origin, 0);
	XMVECTOR OrigY = __MM_REPLICATE(Origin, 1);
	XMVECTOR OrigZ = __MM_REPLICATE(Origin, 2);
	// Splat the already abs Extent for the push out calculation
	XMVECTOR AbsExtentX = __MM_REPLICATE(Extent, 0);
	XMVECTOR AbsExtentY = __MM_REPLICATE(Extent, 1);
	XMVECTOR AbsExtentZ = __MM_REPLICATE(Extent, 2);

	// Calculate the distance (x * x) + (y * y) + (z * z) - w
	XMVECTOR DistX_0 = XMVectorMultiply(OrigX, PlanesX_0);
	XMVECTOR DistY_0 = XMVectorMultiplyAdd(OrigY, PlanesY_0, DistX_0);
	XMVECTOR DistZ_0 = XMVectorMultiplyAdd(OrigZ, PlanesZ_0, DistY_0);
	XMVECTOR Distance_0 = XMVectorSubtract(DistZ_0, PlanesW_0);
	// Now do the push out FMath::Abs(x * x) + FMath::Abs(y * y) + FMath::Abs(z * z)
	XMVECTOR PushX_0 = XMVectorMultiply(AbsExtentX, XMVectorAbs(PlanesX_0));
	XMVECTOR PushY_0 = XMVectorMultiplyAdd(AbsExtentY, XMVectorAbs(PlanesY_0), PushX_0);
	XMVECTOR PushOut_0 = XMVectorMultiplyAdd(AbsExtentZ, XMVectorAbs(PlanesZ_0), PushY_0);

	// Check for completely outside
	if (_mm_movemask_ps(_mm_cmpgt_ps(Distance_0, PushOut_0)))
	{
		return false;
	}

	// Calculate the distance (x * x) + (y * y) + (z * z) - w
	XMVECTOR DistX_1 = XMVectorMultiply(OrigX, PlanesX_1);
	XMVECTOR DistY_1 = XMVectorMultiplyAdd(OrigY, PlanesY_1, DistX_1);
	XMVECTOR DistZ_1 = XMVectorMultiplyAdd(OrigZ, PlanesZ_1, DistY_1);
	XMVECTOR Distance_1 = XMVectorSubtract(DistZ_1, PlanesW_1);
	// Now do the push out FMath::Abs(x * x) + FMath::Abs(y * y) + FMath::Abs(z * z)
	XMVECTOR PushX_1 = XMVectorMultiply(AbsExtentX, XMVectorAbs(PlanesX_1));
	XMVECTOR PushY_1 = XMVectorMultiplyAdd(AbsExtentY, XMVectorAbs(PlanesY_1), PushX_1);
	XMVECTOR PushOut_1 = XMVectorMultiplyAdd(AbsExtentZ, XMVectorAbs(PlanesZ_1), PushY_1);

	// Check for completely outside
	if (_mm_movemask_ps(_mm_cmpgt_ps(Distance_1, PushOut_1)))
	{
		return true;
	}
	return false;
}

#define VecOp1Imp(Dim, Op, a) \
Vector##Dim##f out; \
DirectX::XMStoreFloat##Dim((DirectX::XMFLOAT##Dim*)&out, DirectX::XMVector##Op(DirectX::XMLoadFloat##Dim((DirectX::XMFLOAT##Dim*)&a))); \
return out;

#define VecOp2Imp(Dim, Op, a, b) \
Vector##Dim##f out; \
DirectX::XMStoreFloat##Dim((DirectX::XMFLOAT##Dim*)&out, DirectX::XMVector##Op(DirectX::XMLoadFloat##Dim((DirectX::XMFLOAT##Dim*)&a), DirectX::XMLoadFloat##Dim((DirectX::XMFLOAT##Dim*)&b))); \
return out;

Vector2f Math::abs(const Vector2f& a)
{
	VecOp1Imp(2, Abs, a)
}

Vector2f Math::min(const Vector2f& a, const Vector2f& b)
{
	VecOp2Imp(2, Min, a, b)
}

Vector2f Math::max(const Vector2f& a, const Vector2f& b)
{
	VecOp2Imp(2, Max, a, b)
}

float Math::distance(const Vector2f& a, const Vector2f& b)
{
	return (a - b).norm();
}

Vector3f Math::abs(const Vector3f& a)
{
	VecOp1Imp(3, Abs, a)
}

Vector3f Math::min(const Vector3f& a, const Vector3f& b)
{
	VecOp2Imp(3, Min, a, b)
}

Vector3f Math::max(const Vector3f& a, const Vector3f& b)
{
	VecOp2Imp(3, Max, a, b)
}

float Math::distance(const Vector3f& a, const Vector3f& b)
{
	return (a - b).norm();
}

Vector4f Math::abs(const Vector4f& a)
{
	VecOp1Imp(4, Abs, a)
}

Vector4f Math::min(const Vector4f& a, const Vector4f& b)
{
	VecOp2Imp(4, Min, a, b)
}

Vector4f Math::max(const Vector4f& a, const Vector4f& b)
{
	VecOp2Imp(4, Max, a, b)
}

float Math::distance(const Vector4f& a, const Vector4f& b)
{
	return (a - b).norm();
}

Matrix4f Math::getTransitionMatrix(const Vector3f& position)
{
	Matrix4f T = Matrix4f::Identity();
	T(0, 3) = position.x();
	T(1, 3) = position.y();
	T(2, 3) = position.z();
	return T;
}

Matrix4f Math::getRotationMatrix(const Quaternionf& rotation)
{
	Matrix4f R = Matrix4f::Identity();
	R.block(0, 0, 3, 3) = rotation.toRotationMatrix();
	return R;
}

Matrix4f Math::getScaleMatrix(const Vector3f& scale)
{
	Matrix4f S = Matrix4f::Identity();
	S(0, 0) = scale.x();
	S(1, 1) = scale.y();
	S(2, 2) = scale.z();
	return S;
}

Matrix4f Math::getTransformMatrix(const Vector3f& position, const Quaternionf& rotation, const Vector3f& scale)
{
	return getTransitionMatrix(position) * getRotationMatrix(rotation) * getScaleMatrix(scale);
}
