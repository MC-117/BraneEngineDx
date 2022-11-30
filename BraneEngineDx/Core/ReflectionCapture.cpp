#include "ReflectionCapture.h"
#include "Asset.h"
#include "RenderCore/RenderCore.h"

ReflectionCapture::ReflectionCapture(const string& name)
{
	setRadius(10);
}

TextureCube* ReflectionCapture::getReflectionCubeMap() const
{
	return reflectionCubeMap;
}

void ReflectionCapture::setReflectionCubeMap(TextureCube* reflectionCubeMap)
{
	this->reflectionCubeMap = reflectionCubeMap;
}

Range<Vector3f> ReflectionCapture::getWorldBound() const
{
	return bound;
}

Vector3f ReflectionCapture::getWorldPosition() const
{
	return (bound.maxVal + bound.minVal) * 0.5f;
}

void ReflectionCapture::setWorldPosition(const Vector3f& position)
{
	float radius = getRadius();
	Vector3f radiusVector = { radius, radius, radius };
	bound.minVal = position - radiusVector;
	bound.maxVal = position + radiusVector;
	sceneCaptureCube.worldPosition = position;
}

float ReflectionCapture::getRadius() const
{
	return (bound.maxVal - bound.minVal).x() * 0.5f;
}

void ReflectionCapture::setRadius(float radius)
{
	Vector3f position = (bound.minVal + bound.maxVal) * 0.5f;
	Vector3f radiusVector = { radius, radius, radius };
	bound.minVal = position - radiusVector;
	bound.maxVal = position + radiusVector;
}

int ReflectionCapture::getResolution() const
{
	return resolution;
}

void ReflectionCapture::setResolution(int resolution)
{
	this->resolution = resolution;
}

void ReflectionCapture::updateCapture()
{
	update = true;
	if (reflectionCubeMap == NULL)
		reflectionCubeMap = new TextureCube(resolution, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_UF });
}

int ReflectionCapture::getProbeIndex() const
{
	return probeIndex;
}

void ReflectionCapture::render(RenderInfo& info)
{
	if (reflectionCubeMap == NULL)
		return;
	probeIndex = info.sceneData->setReflectionCapture(this);
	if (update) {
		sceneCaptureCube.setTexture(*reflectionCubeMap);
		sceneCaptureCube.setSize({ resolution, resolution });
		sceneCaptureCube.render(info);
		info.sceneData->reflectionProbeDataPack.cubeMapPool.markRefreshCubeMap(reflectionCubeMap);
		update = false;
	}
}
