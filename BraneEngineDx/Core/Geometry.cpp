#include "Geometry.h"
#include "IVendor.h"
#include "Console.h"
#include "Utility/MathUtility.h"

Geometry::Geometry()
{
}

Geometry::Geometry(Vector3f controlPointA, Vector3f controlPointB)
{
	bound.minPoint = controlPointA;
	bound.maxPoint = controlPointB;
}

void Geometry::reshape(Vector3f controlPointA, Vector3f controlPointB)
{
	bound.minPoint = controlPointA;
	bound.maxPoint = controlPointB;
}

Mesh* Geometry::toMesh()
{
	return NULL;
}

SerializeInstance(Box);

Box::Box(Vector3f controlPointA, Vector3f controlPointB) : Geometry(controlPointA, controlPointB)
{
}

Box::Box(float edgeLength, Vector3f center)
{
	Vector3f h = { edgeLength / 2, edgeLength / 2, edgeLength / 2 };
	bound.minPoint = center - h;
	bound.maxPoint = center + h;
}

Box::Box(float width, float height, float depth, Vector3f center)
{
	Vector3f h = { depth / 2, width / 2, height / 2 };
	bound.minPoint = center - h;
	bound.maxPoint = center + h;
}

Mesh* Box::toMesh()
{
	MeshPart part = VendorManager::getInstance().getVendor().newMeshPart(24, 12);

	for (int i = 0; i < 6; i++) {
		part.uv(i * 4) = { 1, 0 };
		part.uv(i * 4 + 1) = { 0, 0 };
		part.uv(i * 4 + 2) = { 0, 1 };
		part.uv(i * 4 + 3) = { 1, 1 };
	}

	part.vertex(0) = bound.minPoint;
	part.vertex(1) = { bound.minPoint.x(), bound.maxPoint.y(), bound.minPoint.z() };
	part.vertex(2) = { bound.minPoint.x(), bound.maxPoint.y(), bound.maxPoint.z() };
	part.vertex(3) = { bound.minPoint.x(), bound.minPoint.y(), bound.maxPoint.z() };
	for (int i = 0; i < 4; i++)
		part.normal(i) = -Vector3f::UnitX();

	part.vertex(4) = { bound.maxPoint.x(), bound.minPoint.y(), bound.minPoint.z() };
	part.vertex(5) = part.vertex(0);
	part.vertex(6) = part.vertex(3);
	part.vertex(7) = { bound.maxPoint.x(), bound.minPoint.y(), bound.maxPoint.z() };
	for (int i = 4; i < 8; i++)
		part.normal(i) = -Vector3f::UnitY();

	part.vertex(8) = { bound.maxPoint.x(), bound.maxPoint.y(), bound.minPoint.z() };
	part.vertex(9) = part.vertex(4);
	part.vertex(10) = part.vertex(7);
	part.vertex(11) = bound.maxPoint;
	for (int i = 8; i < 12; i++)
		part.normal(i) = Vector3f::UnitX();

	part.vertex(12) = part.vertex(1);
	part.vertex(13) = part.vertex(8);
	part.vertex(14) = part.vertex(11);
	part.vertex(15) = part.vertex(2);
	for (int i = 12; i < 16; i++)
		part.normal(i) = Vector3f::UnitY();

	part.vertex(16) = part.vertex(3);
	part.vertex(17) = part.vertex(15);
	part.vertex(18) = part.vertex(11);
	part.vertex(19) = part.vertex(7);
	for (int i = 16; i < 20; i++)
		part.normal(i) = Vector3f::UnitZ();

	part.vertex(20) = part.vertex(4);
	part.vertex(21) = part.vertex(8);
	part.vertex(22) = part.vertex(12);
	part.vertex(23) = part.vertex(0);
	for (int i = 20; i < 24; i++)
		part.normal(i) = -Vector3f::UnitZ();

	for (unsigned int v = 0, e = 0; v < 24; v += 4, e += 6) {
		part.element(e) = v, part.element(e + 1) = v + 2, part.element(e + 2) = v + 1;
		part.element(e + 3) = v, part.element(e + 4) = v + 3, part.element(e + 5) = v + 2;
	}

	Mesh* mesh = new Mesh();
	mesh->setTotalMeshPart(part);
	mesh->addMeshPart("Materail", part);

	mesh->bound = bound;

	return mesh;
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
	bound.minPoint = center - r;
	bound.maxPoint = center + r;
}

void Sphere::setRadius(float radius)
{
	Vector3f center = getCenter();
	Vector3f r = { radius, radius, radius };
	bound.minPoint = center - r;
	bound.maxPoint = center + r;
}

BoundBox Sphere::getCustomSpaceBound(const Matrix4f& localToCustom)
{
	Vector3f center = getCenter();
	center = Vector3f(localToCustom * Vector4f(center, 1));
	float radius = getRadius();
	Vector3f extend = { radius, radius, radius };
	BoundBox outBound = {
		center - extend,
		center + extend,
	};
	return outBound;
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
	Vector3f r = { radius, radius, height / 2 };
	bound.minPoint = center - r;
	bound.maxPoint = center + r;
}

Mesh* Column::toMesh(unsigned int segment, const Vector3f& axis)
{
	Quaternionf rot = Quaternionf::FromTwoVectors(Vector3f::UnitZ(), axis);

	segment = std::max(segment, 3u);
	Vector3f size = bound.maxPoint - bound.minPoint;
	float radius = min(size.x(), size.y()) / 2;
	float halfLength = size.z() / 2;
	vector<Vector3f> cirlePos = vector<Vector3f>(segment);
	vector<Vector3f> cirleNorm = vector<Vector3f>(segment);
	vector<float> columnU = vector<float>(segment);
	vector<Vector2f> circleUV = vector<Vector2f>(segment);
	for (int s = 0; s < segment; s++) {
		columnU[s] = s / (float)segment * 2;
		float angle = s / (float)segment * PI * 2;
		Vector2f coord = { cos(angle), sin(angle) };
		cirleNorm[s] = Vector3f{ coord.x(), coord.y() }.normalized();
		cirlePos[s] = cirleNorm[s] * radius;
		circleUV[s] = (coord + Vector2f::Ones()) * 0.5;
}

	MeshPart part = VendorManager::getInstance().getVendor().newMeshPart(4 * segment + 2, 4 * segment);

	part.vertex(0) = rot * Vector3f{ 0, 0, halfLength };
	part.normal(0) = rot * Vector3f::UnitZ();
	part.uv(0) = { 0.5, 0.5 };
	part.vertex(1) = rot * Vector3f{ 0, 0, -halfLength };
	part.normal(1) = rot * -Vector3f::UnitZ();
	part.uv(1) = { 0.5, 0.5 };

	unsigned int v = 2, e = 0;
	for (unsigned int i = 0; i < segment; i++, v += 4, e += 12) {
		Vector3f pos = cirlePos[i];
		pos.z() = halfLength;
		Vector3f ipos = cirlePos[i];
		ipos.z() = -halfLength;

		pos = rot * pos;
		ipos = rot * ipos;

		part.vertex(v + 2) = part.vertex(v) = pos;
		part.vertex(v + 3) = part.vertex(v + 1) = ipos;

		part.normal(v + 1) = -(part.normal(v) = rot * Vector3f::UnitZ());
		part.normal(v + 3) = part.normal(v + 2) = rot * cirleNorm[i];

		Vector2f uv = circleUV[i];
		part.uv(v) = part.uv(v + 1) = uv;
		part.uv(v + 2) = { columnU[i], 1 };
		part.uv(v + 3) = { columnU[i], 0 };

		unsigned int indices[12] = {
			v + 4, 0, v,
			v + 5, v + 1, 1,
			v + 7, v + 2, v + 3,
			v + 7, v + 6, v + 2
		};

		unsigned int* dst = &part.element(e);

		memcpy(dst, indices, sizeof(indices));
	}
	e -= 12;
	part.element(e) = 2;
	part.element(e + 3) = 3;
	part.element(e + 6) = 5;
	part.element(e + 9) = 5;
	part.element(e + 10) = 4;

	Mesh* mesh = new Mesh();
	mesh->setTotalMeshPart(part);
	mesh->addMeshPart("Materail", part);

	mesh->bound = { rot * bound.minPoint, rot * bound.maxPoint };

	return mesh;
}

Mesh* Column::toMesh()
{
	return toMesh(36);
}

BoundBox Column::getCustomSpaceBound(const Matrix4f& localToCustom)
{
	float radius = getRadius();
	float height = getHeight();
	Vector3f center = getCenter();
	center = Vector3f(localToCustom * Vector4f(center, 1));
	Vector3f axis = Vector3f(localToCustom * Vector4f(Vector3f::UnitZ(), 0)).normalize();
	Vector3f a = center - axis * height;
	Vector3f b = center + axis * height;
	Vector3f exExtend = Math::abs(axis) * radius;
	BoundBox outBound = {
		Math::min(a, b) - exExtend,
		Math::max(a, b) + exExtend
	};
	return outBound;
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
	bound.minPoint = center - r;
	bound.maxPoint = center + r;
}

BoundBox Cone::getCustomSpaceBound(const Matrix4f& localToCustom)
{
	float radius = getRadius();
	float height = getHeight();
	Vector3f center = getCenter();
	center = Vector3f(localToCustom * Vector4f(center, 1));
	Vector3f axis = Vector3f(localToCustom * Vector4f(Vector3f::UnitZ(), 0)).normalize();
	Vector3f a = center - axis * (height + radius);
	Vector3f b = center + axis * height;
	BoundBox outBound = {
		Math::min(a, b),
		Math::max(a, b)
	};
	return outBound;
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
	Vector3f r = { radius, radius, halfLength + radius };
	bound.minPoint = center - r;
	bound.maxPoint = center + r;
}

void Capsule::setRadius(float radius)
{
	float halfLength = getHeight() * 0.5 - getRadius();
	Vector3f center = getCenter();
	Vector3f r = { radius, radius, halfLength + radius };
	bound.minPoint = center - r;
	bound.maxPoint = center + r;
}

void Capsule::setHalfLength(float halfLength)
{
	float radius = getRadius();
	Vector3f center = getCenter();
	Vector3f r = { radius, radius, halfLength + radius };
	bound.minPoint = center - r;
	bound.maxPoint = center + r;
}

BoundBox Capsule::getCustomSpaceBound(const Matrix4f& localToCustom)
{
	float height = getHeight();
	float radius = getRadius();
	float halfLength = height - radius;
	Vector3f center = getCenter();
	center = Vector3f(localToCustom * Vector4f(center, 1));
	Vector3f axis = Vector3f(localToCustom * Vector4f(Vector3f::UnitZ(), 0)).normalize();
	Vector3f a = center - axis * halfLength;
	Vector3f b = center + axis * halfLength;
	Vector3f radiusV = { radius, radius, radius };
	BoundBox outBound = {
		Math::min(a, b) - radiusV,
		Math::max(a, b) + radiusV
	};
	return outBound;
}

#if ENABLE_PHYSICS
CollisionShape * Capsule::generateCollisionShape(const Vector3f& scale) const
{
#ifdef PHYSICS_USE_BULLET
	return new btCapsuleShapeZ(getRadius() * scale.x(), (getHeight() - getRadius()) * scale.y());
#endif
#ifdef PHYSICS_USE_PHYSX
	return new PxCapsuleGeometry(getRadius() * scale.x(), (getHeight() * 0.5 - getRadius()) * scale.y());
#endif
}
#endif

Serializable * Capsule::instantiate(const SerializationInfo & from)
{
	return new Capsule();
}
