#include "BlitPass.h"
#include "../Engine.h"
#include "../Asset.h"
#include "../Console.h"
#include "../CameraRender.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/SceneRenderData.h"

Material* BlitPass::material = NULL;
ShaderProgram* BlitPass::program = NULL;
bool BlitPass::isInited = false;

void BlitPass::prepare()
{
	loadDefaultResource();
	CameraRender* cameraRender = CameraRender::getMainCameraRender();
	if (cameraRender == NULL)
		return;
	program->init();

	materialRenderData = (MaterialRenderData*)material->getRenderData();
	materialRenderData->program = program;
	materialRenderData->create();
}

void BlitPass::execute(IRenderContext& context)
{
	CameraRender* cameraRender = CameraRender::getMainCameraRender();
	if (cameraRender == NULL)
		return;
	Texture* sceneTexture = cameraRender->getSceneMap();
	if (sceneTexture == NULL)
		return;

	static const ShaderPropertyName screenMapName = "screenMap";

	materialRenderData->upload();

	context.clearFrameBindings();

	context.bindShaderProgram(program);

	context.bindMaterialBuffer(((MaterialRenderData*)materialRenderData)->vendorMaterial);

	SceneRenderData* sceneRenderData  = *renderGraph->sceneDatas.begin();
	sceneRenderData->cameraRenderDatas[0]->bind(context);

	context.bindSurface(Engine::getMainDeviceSurface());
	context.bindTexture((ITexture*)sceneTexture->getVendorTexture(), screenMapName);

	context.setDrawInfo(0, 1, 0);
	context.clearSurfaceColor({ 0, 0, 0, 0 });
	context.setViewport(0, 0, cameraRender->size.x, cameraRender->size.y);
	context.postProcessCall();
}

void BlitPass::reset()
{
}

void BlitPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
	CameraRender* cameraRender = CameraRender::getMainCameraRender();
	if (cameraRender == NULL)
		return;
	Texture* sceneTexture = cameraRender->getSceneMap();
	if (sceneTexture == NULL)
		return;
	textures.push_back({ "ScreenMap", sceneTexture });
}

void BlitPass::loadDefaultResource()
{
	if (isInited)
		return;
	material = getAssetByPath<Material>("Engine/Shaders/PostProcess/BlitPass.mat");
	//material = getAssetByPath<Material>("Engine/Shaders/Editor/VisualProbeGrid.mat");
	program = material->getShader()->getProgram(Shader_Postprocess);
	isInited = true;
}
