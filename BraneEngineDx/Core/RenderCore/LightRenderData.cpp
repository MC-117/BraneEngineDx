#include "LightRenderData.h"
#include "../DirectLight.h"
#include "../PointLight.h"

void LightRenderData::setLight(Light* light)
{
	if (shadowTarget == NULL && light->getShadowRenderTarget() != NULL) {
		shadowTarget = light->getShadowRenderTarget();
	}
	DirectLight* directLight = dynamic_cast<DirectLight*>(light);
	if (directLight != NULL) {
		directLightData.direction = directLight->getForward(WORLD);
		directLightData.intensity = directLight->intensity;
		directLightData.lightSpaceMat = MATRIX_UPLOAD_OP(directLight->getLightSpaceMatrix());
		directLightData.shadowBias = directLight->getShadowBias();
		directLightData.color = Vector3f(directLight->color.r, directLight->color.g, directLight->color.b);
	}
	PointLight* pointLight = dynamic_cast<PointLight*>(light);
	if (pointLight != NULL) {
		PointLightData data;
		data.position = pointLight->getPosition(WORLD);
		data.intensity = pointLight->intensity;
		data.color = Vector3f(pointLight->color.r, pointLight->color.g, pointLight->color.b);
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
	context.bindBufferBase(directLightBuffer.getVendorGPUBuffer(), DIRECT_LIGHT_BIND_INDEX);
	if (directLightData.pointLightCount > 0)
		context.bindBufferBase(pointLightBuffer.getVendorGPUBuffer(), POINT_LIGHT_BIND_INDEX);
}

void LightRenderData::clean()
{
	shadowTarget = NULL;
	directLightData.pointLightCount = 0;
	pointLightDatas.clear();
}