#include "Geometry.h"

Geometry::Geometry()
{
}

Geometry::Geometry(Vector3f controlPointA, Vector3f controlPointB)
{
	bound.minVal = controlPointA;
	bound.maxVal = controlPointB;
}

void Geometry::reshape(Vector3f controlPointA, Vector3f controlPointB)
{
	bound.minVal = controlPointA;
	bound.maxVal = controlPointB;
}

Mesh& Geometry::toMesh(Mesh & mesh)
{
	return mesh;
}

SerializeInstance(Box);

Box::Box(Vector3f controlPointA, Vector3f controlPointB) : Geometry(controlPointA, controlPointB)
{
}

Box::Box(float edgeLength, Vector3f center)
{
	Vector3f h = { edgeLength / 2, edgeLength / 2, edgeLength / 2 };
	bound.minVal = center - h;
	bound.maxVal = center + h;
}

Box::Box(float width, float height, float depth, Vector3f center)
{
	Vector3f h = { depth / 2, width / 2, height / 2 };
	bound.minVal = center - h;
	bound.maxVal = center + h;
}

Serializable * Box::instantiate(const SerializationInfo & from)
{
	return new Box();
}

SerializeInstance(Sphere);

Sphere::Sphere(Vector3f controlPointA, Vector3f controlPointB) : Geometry(controlPointA, controlPointB)
{
}

Sphere::Sphere(float radius, Vector3f center)
{
	Vector3f r = { radius, radius, radius };
	bound.minVal = center - r;
	bound.maxVal = center + r;
}

void Sphere::setRadius(float radius)
{
	Vector3f center = getCenter();
	Vector3f r = { radius, radius, radius };
	bound.minVal = center - r;
	bound.maxVal = center + r;
}

#if ENABLE_PHYSICS
CollisionShape * Sphere::generateCollisionShape(const Vector3f& scale) const
{
#ifdef PHYSICS_USE_BULLET
	return new btSphereShape(getRadius() * scale.x());
#endif
#ifdef PHYSICS_USE_PHYSX
	return new PxSphereGeometry(getRadius() * scale.x());
#endif
}
#endif

Serializable * Sphere::instantiate(const SerializationInfo & from)
{
	return new Sphere();
}

SerializeInstance(Column);

Column::Column(Vector3f controlPointA, Vector3f controlPointB) : Geometry(controlPointA, controlPointB)
{
}

Column::Column(float radius, float height, Vector3f center)
{
	Vector3f r = { radius, height / 2, radius };
	bound.minVal = center - r;
	bound.maxVal = center + r;
}

#if ENABLE_PHYSICS
CollisionShape * Column::generateCollisionShape(const Vector3f& scale) const
{
#ifdef PHYSICS_USE_BULLET
	return new btCylinderShape(PVec3(getDepth() / 2 * scale.x(), getWidth() / 2 * scale.y(), getHeight() / 2 * scale.z()));
#endif
#ifdef PHYSICS_USE_PHYSX
	return Geometry::generateCollisionShape(scale);
#endif
}
#endif

Serializable * Column::instantiate(const SerializationInfo & from)
{
	return new Column();
}

SerializeInstance(Cone);

Cone::Cone(Vector3f controlPointA, Vector3f controlPointB) : Geometry(controlPointA, controlPointB)
{
}

Cone::Cone(float radius, float height, Vector3f center)
{
	Vector3f r = { radius, height / 2, radius };
	bound.minVal = center - r;
	bound.maxVal = center + r;
}

#if ENABLE_PHYSICS
CollisionShape * Cone::generateCollisionShape(const Vector3f& scale) const
{
#ifdef PHYSICS_USE_BULLET
	return new btConeShape(getRadius() * scale.x(), getHeight() * scale.z());
#endif
#ifdef PHYSICS_USE_PHYSX
	return Geometry::generateCollisionShape(scale);
#endif
}
#endif

Serializable * Cone::instantiate(const SerializationInfo & from)
{
	return new Cone();
}

SerializeInstance(Capsule);

Capsule::Capsule(Vector3f controlPointA, Vector3f controlPointB) : Geometry(controlPointA, controlPointB)
{
}

Capsule::Capsule(float radius, float halfLength, Vector3f center)
{
	Vector3f r = { radius, radius, halfLength < radius ? radius : halfLength };
	bound.minVal = center - r;
	bound.maxVal = center + r;
}

void Capsule::setRadius(float radius)
{
	float halfLength = getHeight() / 2.0f;
	radius = radius > halfLength ? halfLength : radius;
	Vector3f center = getCenter();
	Vector3f r = { radius, radius, halfLength };
	bound.minVal = center - r;
	bound.maxVal = center + r;
}

void Capsule::setHalfLength(float halfLength)
{
	float radius = getRadius();
	Vector3f center = getCenter();
	Vector3f r = { radius, radius, halfLength < radius ? radius : halfLength };
	bound.minVal = center - r;
	bound.maxVal = center + r;
}

#if ENABLE_PHYSICS
CollisionShape * Capsule::generateCollisionShape(const Vector3f& scale) const
{
#ifdef PHYSICS_USE_BULLET
	return new btCapsuleShapeZ(getRadius() * scale.x(), getWidth() * scale.y());
#endif
#ifdef PHYSICS_USE_PHYSX
	return new PxCapsuleGeometry(getRadius() * scale.x(), getWidth() * scale.y());
#endif
}
#endif

Serializable * Capsule::instantiate(const SerializationInfo & from)
{
	return new Capsule();
}
