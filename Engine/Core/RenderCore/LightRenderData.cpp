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

LightRenderData::LightRenderData(ProbePoolRenderData& probePool) : probePool(probePool)
{
}

void LightRenderData::setMainLight(Light* light)
{
	if (shadowTarget == NULL && light->getShadowRenderTarget() != NULL) {
		shadowTarget = light->getShadowRenderTarget();
		shadowTarget->init();
	}
	DirectLight* directLight = dynamic_cast<DirectLight*>(light);
	if (directLight != NULL) {
		mainLightData.direction = directLight->getForward(WORLD);
		mainLightData.intensity = directLight->intensity;
		mainLightData.worldToLightView = directLight->getWorldToLightViewMatrix();
		mainLightData.viewOriginToLightView = directLight->getViewOriginToLightViewMatrix();
		mainLightData.viewToLightClip = directLight->getViewToLightClipMatrix();
		mainLightData.worldToLightClip = directLight->getWorldToLightClipMatrix();
		mainLightData.shadowBias = directLight->getShadowBias();
		mainLightData.color = toLinearColor(directLight->color);
		mainLightData.vsmID = -1;
		
		shadowCameraRenderData.data = directLight->shadowData.cameraData;
		shadowCameraRenderData.surface.clearFlags = Clear_Depth;
		shadowCameraRenderData.surface.renderTarget = shadowTarget;
		shadowCameraRenderData.surface.clearColors.resize(1);
		shadowCameraRenderData.surface.clearColors[0] = Color();
		shadowCameraRenderData.renderOrder = 0;
	}
}

int LightRenderData::setLocalLight(Light* light)
{
	if (shadowTarget == NULL && light->getShadowRenderTarget() != NULL) {
		shadowTarget = light->getShadowRenderTarget();
		shadowTarget->init();
	}
	PointLight* pointLight = dynamic_cast<PointLight*>(light);
	if (pointLight != NULL) {
		int probeIndex = -1;
		LocalLightData& data = probePool.emplace(ProbeType::ProbeType_Light, probeIndex).localLightData;
		data.position = pointLight->getPosition(WORLD);
		data.intensity = pointLight->intensity;
		data.color = toLinearColor(pointLight->color);
		data.radius = pointLight->getRadius() * pointLight->getScale(WORLD).x();
		data.vsmID = -1;
		int lightID = pointLightProbeIndices.size();
		pointLightProbeIndices.push_back(probeIndex);
		return lightID;
	}
	return -1;
}

void LightRenderData::create()
{
	mainLightData.pointLightCount = pointLightProbeIndices.size();
}

void LightRenderData::addVirtualShadowMapClipmap(VirtualShadowMapClipmap& clipmap)
{
	mainLightClipmaps.push_back(&clipmap);
}

void LightRenderData::addVirtualShadowMapLocalShadow(VirtualShadowMapLightEntry& localShadow)
{
	localLightShadows.push_back(&localShadow);
}

int LightRenderData::getLocalLightCount() const
{
	return pointLightProbeIndices.size();
}

LocalLightData& LightRenderData::getLocalLightData(int localLightIndex)
{
	return probePool.getProbeByteData(pointLightProbeIndices[localLightIndex]).localLightData;
}

const LocalLightData& LightRenderData::getLocalLightData(int localLightIndex) const
{
	return probePool.getProbeByteData(pointLightProbeIndices[localLightIndex]).localLightData;
}

void LightRenderData::release()
{
}

void LightRenderData::upload()
{
	shadowCameraRenderData.upload();
	MainLightData mainLightDataUpload = mainLightData;
	mainLightDataUpload.worldToLightView = MATRIX_UPLOAD_OP(mainLightDataUpload.worldToLightView);
	mainLightDataUpload.viewToLightClip = MATRIX_UPLOAD_OP(mainLightDataUpload.viewToLightClip);
	mainLightDataUpload.worldToLightClip = MATRIX_UPLOAD_OP(mainLightDataUpload.worldToLightClip);
	mainLightBuffer.uploadData(1, &mainLightDataUpload);
}

void LightRenderData::bind(IRenderContext& context)
{
	static const ShaderPropertyName DirectLightBufferName = "DirectLightBuffer";
	context.bindBufferBase(mainLightBuffer.getVendorGPUBuffer(), DirectLightBufferName); // DIRECT_LIGHT_BIND_INDEX
}

void LightRenderData::clean()
{
	shadowTarget = NULL;
	mainLightData.intensity = 0;
	mainLightData.vsmID = -1;
	mainLightData.pointLightCount = 0;
	pointLightProbeIndices.clear();
	mainLightClipmaps.clear();
	localLightShadows.clear();
}