#include "CaptureProbeRender.h"

SerializeInstance(CaptureProbeRender);

TextureCube* CaptureProbeRender::getProbeCubeMap() const
{
	return nullptr;
}

void CaptureProbeRender::setProbeCubeMap(TextureCube* probeCubeMap)
{
}

Range<Vector3f> CaptureProbeRender::getWorldBound() const
{
	return bound;
}

Vector3f CaptureProbeRender::getWorldPosition() const
{
	return (bound.maxVal + bound.minVal) * 0.5f;
}

void CaptureProbeRender::setWorldPosition(const Vector3f& position)
{
	float radius = getRadius();
	Vector3f radiusVector = { radius, radius, radius };
	bound.minVal = position - radiusVector;
	bound.maxVal = position + radiusVector;
}

float CaptureProbeRender::getRadius() const
{
	return (bound.maxVal - bound.minVal).x() * 0.5f;
}

void CaptureProbeRender::setRadius(float radius)
{
	Vector3f position = (bound.minVal + bound.maxVal) * 0.5f;
	Vector3f radiusVector = { radius, radius, radius };
	bound.minVal = position - radiusVector;
	bound.maxVal = position + radiusVector;
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
	return RenderType::SceneCapture;
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
	return true;
}

bool CaptureProbeRender::serialize(SerializationInfo& to)
{
	serializeInit(this, to);
	to.set("radius", getRadius());
	return true;
}
