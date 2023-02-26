#include "Shape.h"
#include "Utility/MathUtility.h"

SerializeInstance(Shape);

Shape::Shape()
{
}

Shape::Shape(const Shape & shape)
{
	renderMode = shape.renderMode;
	bound = shape.bound;
}

Shape::~Shape()
{
}

Shape & Shape::operator=(const Shape & shape)
{
	renderMode = shape.renderMode;
	bound = shape.bound;
	return *this;
}

float * Shape::data() const
{
	return NULL;
}

unsigned int Shape::bindShape()
{
	return 0;
}

void Shape::drawCall()
{
}

Vector3f Shape::getCenter() const
{
	return bound.getCenter();
}

Vector3f Shape::getExtent() const
{
	return bound.getExtent();
}

Vector3f Shape::getSize() const
{
	return bound.getSize();
}

float Shape::getWidth() const
{
	return abs(bound.maxPoint.y() - bound.minPoint.y());
}

float Shape::getHeight() const
{
	return abs(bound.maxPoint.z() - bound.minPoint.z());
}

float Shape::getDepth() const
{
	return abs(bound.maxPoint.x() - bound.minPoint.x());
}

float Shape::getRadius() const
{
	return abs(bound.maxPoint.x() - bound.minPoint.x()) * 0.5;
}

BoundBox Shape::getCustomSpaceBound(const Matrix4f& localToCustom)
{
	Vector3f center = getCenter();
	Vector3f extend = getExtent() * 2.0f;

	Vector4f corners[8] = {
		Vector4f(bound.minPoint, 1),
		Vector4f(bound.minPoint.x() + extend.x(), bound.minPoint.y(), bound.minPoint.z(), 1),
		Vector4f(bound.minPoint.x(), bound.minPoint.y() + extend.y(), bound.minPoint.z(), 1),
		Vector4f(bound.minPoint.x(), bound.minPoint.y(), bound.minPoint.z() + extend.z(), 1),
		Vector4f(bound.maxPoint.x() - extend.x(), bound.maxPoint.y(), bound.maxPoint.z(), 1),
		Vector4f(bound.maxPoint.x(), bound.maxPoint.y() - extend.y(), bound.maxPoint.z(), 1),
		Vector4f(bound.maxPoint.x(), bound.maxPoint.y(), bound.maxPoint.z() - extend.z(), 1),
		Vector4f(bound.maxPoint, 1)
	};

	for (int i = 0; i < 8; i++) {
		Vector4f& point = corners[i];
		point = localToCustom * point;
	}

	BoundBox outBound = {
		Vector3f(FLT_MAX, FLT_MAX, FLT_MAX),
		Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX)
	};

	DirectX::XMVECTOR outMin = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&outBound.minPoint);
	DirectX::XMVECTOR outMax = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&outBound.maxPoint);

	for (int i = 0; i < 8; i++) {
		DirectX::XMVECTOR corner = DirectX::XMLoadFloat4((DirectX::XMFLOAT4*)&corners[i]);
		outMin = DirectX::XMVectorMin(corner, outMin);
		outMax = DirectX::XMVectorMax(corner, outMax);
	}

	DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&outBound.minPoint, outMin);
	DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&outBound.maxPoint, outMax);

	return outBound;
}

#if ENABLE_PHYSICS
CollisionShape * Shape::generateCollisionShape(const Vector3f& scale) const
{
#ifdef PHYSICS_USE_BULLET
	return new btBoxShape(PVec3(getDepth() * 0.5 * scale.x(), getWidth() * 0.5 * scale.y(), getHeight() * 0.5 * scale.z()));
#endif
#ifdef PHYSICS_USE_PHYSX
	return new PxBoxGeometry(PVec3(getDepth() * 0.5 * scale.x(), getWidth() * 0.5 * scale.y(), getHeight() * 0.5 * scale.z()));
#endif
}

CollisionShape * Shape::generateComplexCollisionShape(const Vector3f& scale)
{
	return NULL;
}
#endif

Serializable * Shape::instantiate(const SerializationInfo & from)
{
	return new Shape();
}

bool Shape::deserialize(const SerializationInfo & from)
{
	SVector3f minPoint;
	if (!from.get("minPoint", minPoint))
		return false;
	SVector3f maxPoint;
	if (!from.get("maxPoint", maxPoint))
		return false;
	bound.minPoint = minPoint;
	bound.maxPoint = maxPoint;
	return true;
}

bool Shape::serialize(SerializationInfo & to)
{
	serializeInit(this, to);
	to.set("minPoint", SVector3f(bound.minPoint));
	to.set("maxPoint", SVector3f(bound.maxPoint));
	return true;
}
