#include "EnvLightCaptureProbeRender.h"
#include "../RenderCore/RenderCore.h"

SerializeInstance(EnvLightCaptureProbeRender);

EnvLightCaptureProbeRender::EnvLightCaptureProbeRender()
{
	resolution = 32;
	setRadius(200);
}

TextureCube* EnvLightCaptureProbeRender::getLightCubeMap() const
{
	return lightCubeMap;
}

void EnvLightCaptureProbeRender::setLightCubeMap(TextureCube* lightCubeMap)
{
	this->lightCubeMap = lightCubeMap;
}

void EnvLightCaptureProbeRender::setWorldPosition(const Vector3f& position)
{
	CaptureProbeRender::setWorldPosition(position);
	sceneCaptureCube.worldPosition = position;
}

void EnvLightCaptureProbeRender::updateCapture()
{
	update = true;
	if (lightCubeMap == NULL)
		lightCubeMap = new TextureCube(resolution, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_UF });
}

int EnvLightCaptureProbeRender::getProbeIndex() const
{
	return lightDataIndex;
}

int EnvLightCaptureProbeRender::getCaptureIndex() const
{
	return captureIndex;
}

void EnvLightCaptureProbeRender::render(RenderInfo& info)
{
	if (update) {
		if (lightCubeMap) {
			sceneCaptureCube.setTexture(*lightCubeMap);
			sceneCaptureCube.setSize({ resolution, resolution });
			sceneCaptureCube.render(info);
			captureIndex = info.sceneData->setEnvLightCapture(this);
		}
		update = false;
	}
	lightDataIndex = info.sceneData->setEnvLightData(this);
}

Serializable* EnvLightCaptureProbeRender::instantiate(const SerializationInfo& from)
{
	return new EnvLightCaptureProbeRender();
}

bool EnvLightCaptureProbeRender::deserialize(const SerializationInfo& from)
{
	if (!CaptureProbeRender::deserialize(from))
		return false;
	return true;
}

bool EnvLightCaptureProbeRender::serialize(SerializationInfo& to)
{
	if (!CaptureProbeRender::serialize(to))
		return false;
	return true;
}
