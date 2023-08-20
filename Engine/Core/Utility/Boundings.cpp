#include "Boundings.h"
#include "MathUtility.h"

BoundBox BoundBox::none = {
	{ FLT_MAX, FLT_MAX, FLT_MAX },
	{ -FLT_MAX, -FLT_MAX, -FLT_MAX },
};

BoundBox::BoundBox(const Vector3f& minPoint, const Vector3f& maxPoint)
	: minPoint(minPoint), maxPoint(maxPoint)
{
}

BoundBox::BoundBox(const ExtentBox& extentBox)
	: minPoint(extentBox.getMinPoint()), maxPoint(extentBox.getMaxPoint())
{
}

bool BoundBox::isNone() const
{
	return minPoint.x() > maxPoint.x() ||
		minPoint.y() > maxPoint.y() ||
		minPoint.z() > maxPoint.z();
}

Vector3f BoundBox::getCenter() const
{
	return (minPoint + maxPoint) * 0.5f;
}

Vector3f BoundBox::getExtent() const
{
	return (maxPoint - minPoint) * 0.5f;
}

Vector3f BoundBox::getSize() const
{
	return maxPoint - minPoint;
}

void BoundBox::encapsulate(const BoundBox& box)
{
	if (!box.isNone()) {
		minPoint = Math::min(minPoint, box.minPoint);
		maxPoint = Math::max(maxPoint, box.maxPoint);
	}
}

void BoundBox::encapsulate(const Vector3f& point)
{
	minPoint = Math::min(minPoint, point);
	maxPoint = Math::max(maxPoint, point);
}

bool BoundBox::isInBox(const Vector3f& point) const
{
	return minPoint.x() <= point.x() && point.x() <= maxPoint.x()
		&& minPoint.y() <= point.y() && point.y() <= maxPoint.y()
		&& minPoint.z() <= point.z() && point.z() <= maxPoint.z();
}

Vector4f BoundBox::getBoundSphere() const
{
	return Vector4f(getCenter(), Math::distance(minPoint, maxPoint) * 0.5f);
}

bool BoundBox::operator==(const BoundBox& box) const
{
	return minPoint == box.minPoint && maxPoint == box.maxPoint;
}

bool BoundBox::operator!=(const BoundBox& box) const
{
	return minPoint != box.minPoint || maxPoint != box.maxPoint;
}

ExtentBox ExtentBox::none = {
	{ 0, 0, 0 },
	{ -FLT_MAX, -FLT_MAX, -FLT_MAX },
};

ExtentBox::ExtentBox(const Vector3f& center, const Vector3f& extent)
	: center(center), extent(extent)
{
}

ExtentBox::ExtentBox(const BoundBox& boundBox)
	: center(boundBox.getCenter()), extent(boundBox.getExtent())
{
}

bool ExtentBox::isNone() const
{
	return extent.x() < 0 || extent.y() < 0 || extent.z() < 0;
}

Vector3f ExtentBox::getMinPoint() const
{
	return center - extent;
}

Vector3f ExtentBox::getMaxPoint() const
{
	return center + extent;
}

bool ExtentBox::isInBox(const Vector3f& point) const
{
	Vector3f diff = Math::abs(point - center);
	return diff.x() < extent.x()
		&& diff.y() < extent.y()
		&& diff.z() < extent.z();
}

Vector4f ExtentBox::getBoundSphere() const
{
	return Vector4f(center, extent.norm());
}

bool ExtentBox::operator==(const ExtentBox& box) const
{
	return center == box.center && extent == box.extent;
}

bool ExtentBox::operator!=(const ExtentBox& box) const
{
	return center != box.center || extent != box.extent;
}
