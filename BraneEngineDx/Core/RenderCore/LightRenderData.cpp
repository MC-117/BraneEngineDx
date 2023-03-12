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
		mainLightData.direction = directLight->getForward(WORLD);
		mainLightData.intensity = directLight->intensity;
		mainLightData.worldToLightView = MATRIX_UPLOAD_OP(directLight->getWorldToLightViewMatrix());
		mainLightData.viewToLightClip = MATRIX_UPLOAD_OP(directLight->getViewToLightClipMatrix());
		mainLightData.worldToLightClip = MATRIX_UPLOAD_OP(directLight->getWorldToLightClipMatrix());
		mainLightData.shadowBias = directLight->getShadowBias();
		mainLightData.color = toLinearColor(directLight->color);
		shadowCameraRenderData.data = directLight->shadowData.cameraData;
		shadowCameraRenderData.surface.clearFlags = Clear_Depth;
		shadowCameraRenderData.surface.renderTarget = shadowTarget;
		shadowCameraRenderData.surface.clearColors.resize(1);
		shadowCameraRenderData.surface.clearColors[0] = Color();
		shadowCameraRenderData.renderOrder = 0;
	}
	PointLight* pointLight = dynamic_cast<PointLight*>(light);
	if (pointLight != NULL) {
		LocalLightData data;
		data.position = pointLight->getPosition(WORLD);
		data.intensity = pointLight->intensity;
		data.color = toLinearColor(pointLight->color);
		data.radius = pointLight->getRadius();
		pointLightDatas.emplace_back(data);
	}
}

void LightRenderData::create()
{
	mainLightData.pointLightCount = pointLightDatas.size();
}

void LightRenderData::release()
{
}

void LightRenderData::upload()
{
	mainLightBuffer.uploadData(1, &mainLightData);
	if (mainLightData.pointLightCount > 0)
		localLightBuffer.uploadData(mainLightData.pointLightCount, pointLightDatas.data());
}

void LightRenderData::bind(IRenderContext& context)
{
	static const ShaderPropertyName DirectLightBufferName = "DirectLightBuffer";
	static const ShaderPropertyName pointLightsName = "pointLights";
	context.bindBufferBase(mainLightBuffer.getVendorGPUBuffer(), DirectLightBufferName); // DIRECT_LIGHT_BIND_INDEX
	if (mainLightData.pointLightCount > 0)
		context.bindBufferBase(localLightBuffer.getVendorGPUBuffer(), pointLightsName); // POINT_LIGHT_BIND_INDEX
}

void LightRenderData::clean()
{
	shadowTarget = NULL;
	mainLightData.pointLightCount = 0;
	pointLightDatas.clear();
}