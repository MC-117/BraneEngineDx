#include "VirtualShadowMapRenderData.h"
#include "../../CameraRender.h"

VirtualShadowMapRenderData::VirtualShadowMapRenderData(LightRenderData& lightRenderData)
	: lightRenderData(lightRenderData)
{
	shadowMapArray.init(manager);
}

VirtualShadowMapClipmap* VirtualShadowMapRenderData::newClipmap(CameraRender& cameraRender)
{
	return new VirtualShadowMapClipmap(
		shadowMapArray,
		*cameraRender.getRenderData(),
		lightRenderData.mainLightData);
}

void VirtualShadowMapRenderData::create()
{
	manager.prepare();
}

void VirtualShadowMapRenderData::release()
{
	manager.invalidate();
}

void VirtualShadowMapRenderData::upload()
{
	intanceIndexArray.processIndices();
}

void VirtualShadowMapRenderData::clean()
{
	shadowMapArray.clean();
	manager.swapFrameData(shadowMapArray);
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