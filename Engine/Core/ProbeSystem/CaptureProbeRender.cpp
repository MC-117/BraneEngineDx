#include "CaptureProbeRender.h"

SerializeInstance(CaptureProbeRender);

TextureCube* CaptureProbeRender::getProbeCubeMap() const
{
	return nullptr;
}

void CaptureProbeRender::setProbeCubeMap(TextureCube* probeCubeMap)
{
}

BoundBox CaptureProbeRender::getWorldBound() const
{
	return bound;
}

Vector3f CaptureProbeRender::getWorldPosition() const
{
	return bound.getCenter();
}

float CaptureProbeRender::getWorldScale() const
{
	return getWorldRadius() / getRadius();
}

void CaptureProbeRender::setWorldPositionAndScale(const Vector3f& position, float scale)
{
	float radius = getRadius() * scale;
	Vector3f radiusVector = { radius, radius, radius };
	bound.minPoint = position - radiusVector;
	bound.maxPoint = position + radiusVector;
}

float CaptureProbeRender::getRadius() const
{
	return localRadius;
}

float CaptureProbeRender::getWorldRadius() const
{
	return (bound.maxPoint.x() - bound.minPoint.x()) * 0.5f;
}

void CaptureProbeRender::setRadius(float radius)
{
	float scale = getWorldScale();
	localRadius = radius;
	radius *= scale;
	Vector3f position = bound.getCenter();
	Vector3f radiusVector = { radius, radius, radius };
	bound.minPoint = position - radiusVector;
	bound.maxPoint = position + radiusVector;
}

void CaptureProbeRender::setWorldRadius(float radius)
{
	localRadius = radius / getWorldScale();
	Vector3f position = bound.getCenter();
	Vector3f radiusVector = { radius, radius, radius };
	bound.minPoint = position - radiusVector;
	bound.maxPoint = position + radiusVector;
}

int CaptureProbeRender::getResolution() const
{
	return resolution;
}

void CaptureProbeRender::setResolution(int resolution)
{
	this->resolution = resolution;
}

void CaptureProbeRender::updateCapture()
{
}

int CaptureProbeRender::getProbeIndex() const
{
	return -1;
}

IRendering::RenderType CaptureProbeRender::getRenderType() const
{
	return RenderType::SceneCapture_Render;
}

void CaptureProbeRender::render(RenderInfo& info)
{
}

Serializable* CaptureProbeRender::instantiate(const SerializationInfo& from)
{
	return new CaptureProbeRender();
}

bool CaptureProbeRender::deserialize(const SerializationInfo& from)
{
	float radius = 0;
	if (from.get("radius", radius))
		setRadius(radius);
	int resolution = 128;
	if (from.get("resolution", resolution))
		setResolution(resolution);
	return true;
}

bool CaptureProbeRender::serialize(SerializationInfo& to)
{
	serializeInit(this, to);
	to.set("radius", getRadius());
	to.set("resolution", getResolution());
	return true;
}
