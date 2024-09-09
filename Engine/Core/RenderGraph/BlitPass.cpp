#include "BlitPass.h"
#include "../Engine.h"
#include "../Asset.h"
#include "../Console.h"
#include "../CameraRender.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/SceneRenderData.h"
#include "../GUI/GUISurface.h"

MaterialRenderData* BlitPass::materialRenderData = NULL;
IMaterial* BlitPass::materialVariant = NULL;

bool BlitPass::loadDefaultResource()
{
	if (materialRenderData == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/PostProcess/BlitPass.mat");
		if (material)
			materialRenderData = material->getMaterialRenderData();
	}
	if (materialRenderData)
		renderGraph->getRenderDataCollectorMainThread()->add(*materialRenderData);
	return materialRenderData;
}

void BlitPass::prepare()
{
	const CameraRender* cameraRender = GUISurface::getFullScreenGUISurface().getCameraRender();
	if (cameraRender == NULL)
		return;
	
	materialVariant = materialRenderData->getVariant(Shader_Postprocess);
	if (materialVariant == NULL)
		throw runtime_error("Load Engine/Shaders/PostProcess/BlitPass.mat failed");
	materialVariant->init();
}

void BlitPass::execute(IRenderContext& context)
{
	CameraRender* cameraRender = GUISurface::getFullScreenGUISurface().getCameraRender();
	
	if (cameraRender == NULL) {
		context.bindSurface(Engine::getMainDeviceSurface());
		context.clearSurfaceColor({ 0, 0, 0, 0 });
	}
	else {
		Texture* sceneTexture = cameraRender->getSceneTexture();
		static const ShaderPropertyName screenMapName = "screenMap";

		materialRenderData->upload();

		context.clearFrameBindings();

		context.bindShaderProgram(materialVariant->program);

		context.bindMaterialBuffer(materialVariant);

		SceneRenderData* sceneRenderData  = *renderGraph->sceneDatas.begin();
		sceneRenderData->cameraRenderDatas[0]->bind(context);

		context.bindSurface(Engine::getMainDeviceSurface());
		context.bindTexture((ITexture*)sceneTexture->getVendorTexture(), screenMapName);

		context.setDrawInfo(0, 1, 0);
		context.clearSurfaceColor({ 0, 0, 0, 0 });
		context.setViewport(0, 0, cameraRender->size.x, cameraRender->size.y);
		context.postProcessCall();
	}
}

void BlitPass::reset()
{
}

void BlitPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
	CameraRender* cameraRender = GUISurface::getFullScreenGUISurface().getCameraRender();
	if (cameraRender == NULL)
		return;
	Texture* sceneTexture = cameraRender->getSceneTexture();
	if (sceneTexture == NULL)
		return;
	textures.push_back({ "ScreenMap", sceneTexture });
}
