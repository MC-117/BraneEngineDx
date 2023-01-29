#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#include "Serialization.h"

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
	Range<Vector3f> bound;

	Shape();
	Shape(const Shape& shape);
	virtual ~Shape();

	Shape& operator=(const Shape& shape);

	float* data() const;

	virtual unsigned int bindShape();
	virtual void drawCall();

	virtual Vector3f getCenter() const;
	virtual float getWidth() const;
	virtual float getHeight() const;
	virtual float getDepth() const;
	virtual float getRadius() const;

#if ENABLE_PHYSICS
	virtual CollisionShape* generateCollisionShape(const Vector3f& scale = Vector3f(1, 1, 1)) const;
	virtual CollisionShape* generateComplexCollisionShape(const Vector3f& scale = Vector3f(1, 1, 1));
#endif

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

#endif // !_SHAPE_H_
