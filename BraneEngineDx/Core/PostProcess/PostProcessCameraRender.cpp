#include "PostProcessCameraRender.h"
#include "../Console.h"

PostProcessCameraRender::PostProcessCameraRender(RenderTarget & renderTarget, Material & material) : CameraRender(renderTarget, material)
{
	graph.addDefaultPasses();
}

PostProcessCameraRender::~PostProcessCameraRender()
{
}

Texture2D * PostProcessCameraRender::getSceneBlurTex()
{
	//return &screenBlurYTex;
	return dynamic_cast<Texture2D*>(graph.resource.blurTexture);
}

void PostProcessCameraRender::setSize(Unit2Di size)
{
	if (size != this->size) {
		IRenderContext& context = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
		context.clearFrameBindings();
		if (!renderTarget.isDefault()) {
			renderTarget.resize(size.x, size.y);
		}
		postRenderTarget.resize(size.x, size.y);
		graph.resize(size);
		this->size = size;
	}
}

void PostProcessCameraRender::setVolumnicLight(DirectLight & light)
{
	volumnicLight = &light;
}

void PostProcessCameraRender::render(RenderInfo & info)
{
	DrawElementsIndirectCommand cmd = { 4, 1, 0, 0 };
	if (isValid() /*&& !material.isNull()*/) {
		info.renderGraph->setMainRenderTarget(renderTarget);
		graph.resource.reset();
		graph.resource.depthTexture = renderTarget.getInternalDepthTexture();
		RenderTarget* shadowTarget = info.sceneData->lightDataPack.shadowTarget;
		if (shadowTarget == NULL)
			graph.resource.lightDepthTexture = NULL;
		else
			graph.resource.lightDepthTexture = shadowTarget->getDepthTexture();
		graph.resource.screenTexture = renderTarget.getTexture(0);
		postRenderTarget.addTexture("screenMap", *graph.resource.screenTexture);
		graph.resource.screenRenderTarget = &postRenderTarget;
		graph.render(info);
	}
}
