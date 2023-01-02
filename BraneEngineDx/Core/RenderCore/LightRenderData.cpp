#include "LightRenderData.h"
#include "../DirectLight.h"
#include "../PointLight.h"

Vector3f toLinearColor(const Color& color)
{
	return Vector3f{
		pow(color.r, 2.2f),
		pow(color.g, 2.2f),
		pow(color.b, 2.2f),
	};
}

void LightRenderData::setLight(Light* light)
{
	if (shadowTarget == NULL && light->getShadowRenderTarget() != NULL) {
		shadowTarget = light->getShadowRenderTarget();
		shadowTarget->init();
	}
	DirectLight* directLight = dynamic_cast<DirectLight*>(light);
	if (directLight != NULL) {
		directLightData.direction = directLight->getForward(WORLD);
		directLightData.intensity = directLight->intensity;
		directLightData.lightSpaceMat = MATRIX_UPLOAD_OP(directLight->getLightSpaceMatrix());
		directLightData.shadowBias = directLight->getShadowBias();
		directLightData.color = toLinearColor(directLight->color);
		shadowCameraRenderData.data = directLight->shadowData.cameraData;
		shadowCameraRenderData.surface.clearFlags = Clear_Depth;
		shadowCameraRenderData.surface.renderTarget = shadowTarget;
		shadowCameraRenderData.surface.clearColors.resize(1);
		shadowCameraRenderData.surface.clearColors[0] = Color();
		shadowCameraRenderData.renderOrder = 0;
	}
	PointLight* pointLight = dynamic_cast<PointLight*>(light);
	if (pointLight != NULL) {
		PointLightData data;
		data.position = pointLight->getPosition(WORLD);
		data.intensity = pointLight->intensity;
		data.color = toLinearColor(pointLight->color);
		data.radius = pointLight->getRadius();
		pointLightDatas.emplace_back(data);
	}
}

void LightRenderData::create()
{
	directLightData.pointLightCount = pointLightDatas.size();
}

void LightRenderData::release()
{
}

void LightRenderData::upload()
{
	directLightBuffer.uploadData(1, &directLightData);
	if (directLightData.pointLightCount > 0)
		pointLightBuffer.uploadData(directLightData.pointLightCount, pointLightDatas.data());
}

void LightRenderData::bind(IRenderContext& context)
{
	context.bindBufferBase(directLightBuffer.getVendorGPUBuffer(), "DirectLightBuffer"); // DIRECT_LIGHT_BIND_INDEX
	if (directLightData.pointLightCount > 0)
		context.bindBufferBase(pointLightBuffer.getVendorGPUBuffer(), "pointLights"); // POINT_LIGHT_BIND_INDEX
}

void LightRenderData::clean()
{
	shadowTarget = NULL;
	directLightData.pointLightCount = 0;
	pointLightDatas.clear();
}