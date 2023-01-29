#include "Shape.h"

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
	return (bound.maxVal + bound.minVal) * 0.5f;
}

float Shape::getWidth() const
{
	return abs(bound.maxVal.y() - bound.minVal.y());
}

float Shape::getHeight() const
{
	return abs(bound.maxVal.z() - bound.minVal.z());
}

float Shape::getDepth() const
{
	return abs(bound.maxVal.x() - bound.minVal.x());
}

float Shape::getRadius() const
{
	return abs(bound.maxVal.x() - bound.minVal.x()) * 0.5;
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
	bound.minVal = minPoint;
	bound.maxVal = maxPoint;
	return true;
}

bool Shape::serialize(SerializationInfo & to)
{
	serializeInit(this, to);
	to.set("minPoint", SVector3f(bound.minVal));
	to.set("maxPoint", SVector3f(bound.maxVal));
	return true;
}
