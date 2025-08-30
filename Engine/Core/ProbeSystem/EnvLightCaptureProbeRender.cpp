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

void EnvLightCaptureProbeRender::setWorldPositionAndScale(const Vector3f& position, float scale)
{
	CaptureProbeRender::setWorldPositionAndScale(position, scale);
	sceneCaptureCube.worldPosition = position;
}

void EnvLightCaptureProbeRender::updateCapture()
{
	update = true;
	if (lightCubeMap == NULL) {
		lightCubeMap = new TextureCube(resolution, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA16_FF });
		lightCubeMap->setAutoGenMip(false);
	}
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
	if (!needWarmup && update) {
		if (lightCubeMap) {
			sceneCaptureCube.setTexture(*lightCubeMap);
			sceneCaptureCube.setSize({ resolution, resolution });
			sceneCaptureCube.render(info);
			EnvLightProbeData data;
			data.lightCubeMap = getLightCubeMap();
			data.shCoeff3RGB = &shCoeff3RGB;
			RENDER_THREAD_ENQUEUE_TASK(EnvLightProbeRenderDataUpdate, ([this, data] (RenderThreadContext& context)
			{
				captureIndex = context.sceneRenderData->setEnvLightCapture(data);
			}));
		}
		update = false;
	}
	EnvLightUpdateData updateData;
	updateData.tintColor = Vector3f(tintColor.r, tintColor.g, tintColor.b);
	updateData.position = getWorldPosition();
	updateData.radius = getWorldRadius();
	updateData.cutoff = cutoff;
	updateData.falloff = falloff;
	updateData.shCoeff3RGB = shCoeff3RGB;
	RENDER_THREAD_ENQUEUE_TASK(EnvLightDataUpdate, ([this, updateData] (RenderThreadContext& context)
	{
		lightDataIndex = context.sceneRenderData->setEnvLightData(updateData);
	}));
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
