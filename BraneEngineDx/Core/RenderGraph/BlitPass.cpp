#include "BlitPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../CameraRender.h"
#include "../GUI/UIControl.h"

Material* BlitPass::material = NULL;
ShaderProgram* BlitPass::program = NULL;
bool BlitPass::isInited = false;

void BlitPass::prepare()
{
	loadDefaultResource();
	CameraRender* cameraRender = CameraRender::getMainCameraRender();
	if (cameraRender == NULL)
		return;
	RenderTarget::defaultRenderTarget.resize(cameraRender->size.x, cameraRender->size.y);
	program->init();
	
	screenMapSlot = program->getAttributeOffset("screenMap").offset;
	screenMapSamplerSlot = program->getAttributeOffset("screenMapSampler").offset;

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
	materialRenderData->upload();

	context.clearFrameBindings();

	context.bindShaderProgram(program);

	context.bindMaterialBuffer(((MaterialRenderData*)materialRenderData)->vendorMaterial);

	context.bindFrame(IRenderTarget::defaultRenderTarget);
	context.bindTexture((ITexture*)sceneTexture->getVendorTexture(), Fragment_Shader_Stage, screenMapSlot, screenMapSamplerSlot);

	context.setDrawInfo(0, 1, 0);
	context.clearFrameColor({ 0, 0, 0, 0 });
	context.clearFrameDepth(1);
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
	program = material->getShader()->getProgram(Shader_Postprocess);
	isInited = true;
}