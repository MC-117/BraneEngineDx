#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#include "Serialization.h"
#include "Utility/Boundings.h"

enum ShapeTopologyType
{
	STT_Point, STT_Line, STT_Triangle, STT_TriangleStrip, STT_Quad
};

enum ShapeComplexType
{
	SIMPLE, COMPLEX, NONESHAPE
};

class Shape : public Serializable
{
public:
	Serialize(Shape,);

	ShapeTopologyType renderMode = STT_Triangle;
	BoundBox bound;

	Shape();
	Shape(const Shape& shape);
	virtual ~Shape();

	Shape& operator=(const Shape& shape);

	float* data() const;

	virtual unsigned int bindShape();
	virtual void drawCall();

	virtual Vector3f getCenter() const;
	virtual Vector3f getExtent() const;
	virtual Vector3f getSize() const;
	virtual float getWidth() const;
	virtual float getHeight() const;
	virtual float getDepth() const;
	virtual float getRadius() const;

	virtual BoundBox getCustomSpaceBound(const Matrix4f& localToCustom);

#if ENABLE_PHYSICS
	virtual CollisionShape* generateCollisionShape(const Vector3f& scale = Vector3f(1, 1, 1)) const;
	virtual CollisionShape* generateComplexCollisionShape(const Vector3f& scale = Vector3f(1, 1, 1));
	virtual PTransform getOffsetTransform(const Vector3f& positionOffset, const Quaternionf& rotationOffset, const Vector3f& localScale) const;
#endif

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

#endif // !_SHAPE_H_
