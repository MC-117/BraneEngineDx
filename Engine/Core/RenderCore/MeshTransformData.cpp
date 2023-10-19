#include "MeshTransformData.h"

MeshTransformData::MeshTransformData()
	: localToWorld(Matrix4f::Identity())
	, worldScale(Vector3f::Ones())
	, boundScale(1)
	, localCenter(Vector3f::Zero())
	, localRadius(0)
	, localExtent(Vector3f::Zero())
	, flag(0)
	, updateFrame(0)
{
}

bool MeshTransformData::operator==(const MeshTransformData& other) const
{
	return localToWorld == other.localToWorld &&
		localCenter == other.localCenter &&
		localRadius == other.localRadius &&
		localExtent == other.localExtent &&
		flag == other.flag;
}

bool MeshTransformData::operator!=(const MeshTransformData& other) const
{
	return localToWorld != other.localToWorld ||
		localCenter != other.localCenter ||
		localRadius != other.localRadius ||
		localExtent != other.localExtent ||
		flag != other.flag;
}

bool MeshTransformData::isDynamic() const
{
	return !!(flag & MeshTransform_Dynamic);
}

bool MeshTransformData::isNegativeScale() const
{
	return !!(flag & MeshTransform_NegScale);
}
