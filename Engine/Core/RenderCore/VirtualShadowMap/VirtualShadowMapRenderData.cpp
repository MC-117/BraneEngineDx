#include "VirtualShadowMapRenderData.h"
#include "../../CameraRender.h"

VirtualShadowMapRenderData::VirtualShadowMapRenderData(LightRenderData& lightRenderData)
	: lightRenderData(lightRenderData)
{
	shadowMapArray.init(manager);
}

VirtualShadowMapClipmap* VirtualShadowMapRenderData::newClipmap(CameraRenderData* cameraRenderData)
{
	if (cameraRenderData == NULL) {
		throw runtime_error("cameraRenderData is invalid");
		return NULL;
	}
	return new VirtualShadowMapClipmap(
		shadowMapArray,
		*cameraRenderData,
		lightRenderData.mainLightData);
}

VirtualShadowMapLightEntry* VirtualShadowMapRenderData::newLocalLightShadow(int renderLightID, int persistentLightID)
{
	LocalLightData& lightData = lightRenderData.getLocalLightData(renderLightID);
	VirtualShadowMapManager::LightEntry* lightEntry = manager.setLightEntry(persistentLightID, 0);
	if (lightEntry) {
		lightEntry->updateLocal(lightData);
		for (int i = 0; i < CubeFace::CF_Faces; i++) {
			VirtualShadowMap* shadowMap = shadowMapArray.allocate();
			VirtualShadowMapManager::ShadowMap* shadowEntry = lightEntry->setShadowMap(i);
			shadowEntry->updateLocal(shadowMap->vsmID, *lightEntry);
			shadowMap->shadowMap = shadowEntry;
			if (i == 0)
				lightData.vsmID = shadowMap->vsmID;
		}
	}
	return (VirtualShadowMapLightEntry*)lightEntry;
}

void VirtualShadowMapRenderData::create()
{
	manager.prepare();
	shadowMapArray.init(manager);
}

void VirtualShadowMapRenderData::release()
{
	manager.invalidate();
}

void VirtualShadowMapRenderData::upload()
{
	meshBatchDrawCallCollection.calculateCountAndOffset();
}

void VirtualShadowMapRenderData::clean()
{
	meshBatchDrawCallCollection.clean();
	shadowMapArray.clean();
	manager.swapFrameData(shadowMapArray);
}

void VirtualShadowMapRenderData::bind(IRenderContext& context)
{
	bindForLighting(context);
}

void VirtualShadowMapRenderData::bindForLighting(IRenderContext& context)
{
	if (shadowMapArray.isAllocated()) {
		context.bindTexture(shadowMapArray.physPagePool->getVendorTexture(), VirtualShadowMapShaders::physPagePoolName);
		context.bindBufferBase(shadowMapArray.frameData->projData.getVendorGPUBuffer(), VirtualShadowMapShaders::projDataName);
		context.bindBufferBase(shadowMapArray.frameData->pageTable.getVendorGPUBuffer(), VirtualShadowMapShaders::pageTableName);
		context.bindBufferBase(shadowMapArray.frameData->vsmInfo.getVendorGPUBuffer(), VirtualShadowMapShaders::VSMInfoBuffName);
	}
}
