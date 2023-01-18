#include "ImagePass.h"
#include "../Asset.h"
#include "../RenderCore/RenderCore.h"

ImagePass::ImagePass(const string& name, Material* material) : PostProcessPass(name, material)
{
	screenMap.setAutoGenMip(false);
	screenRenderTarget.addTexture("image", screenMap);
}

void ImagePass::prepare()
{
	Texture2D* sceneMap = dynamic_cast<Texture2D*>(resource->screenTexture);
	if (sceneMap)
		screenMap.setTextureInfo(sceneMap->getTextureInfo());
	screenRenderTarget.init();
	MaterialRenderData* materialRenderData = (MaterialRenderData*)this->materialRenderData;
	materialRenderData->program = program;
	materialRenderData->create();
}

void ImagePass::execute(IRenderContext& context)
{
	context.bindShaderProgram(program);
	context.bindFrame(screenRenderTarget.getVendorRenderTarget());
	context.bindMaterialTextures(((MaterialRenderData*)materialRenderData)->vendorMaterial);
	context.setRenderPostState();
	context.postProcessCall();
	context.clearFrameBindings();
	resource->screenTexture = &screenMap;
	resource->screenRenderTarget = &screenRenderTarget;
}

bool ImagePass::mapMaterialParameter(RenderInfo& info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/Editor/ImagePPTest.mat");
	if (material == NULL || resource == NULL || resource->screenTexture == NULL)
		return false;
	image = *material->getTexture("image");
	materialRenderData = material->getRenderData();
	return image && materialRenderData;
}

void ImagePass::render(RenderInfo& info)
{
	if (!enable)
		return;
	if (!mapMaterialParameter(info))
		return;
	if (size.x == 0 || size.y == 0)
		return;
	program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL) {
		return;
	}

	program->init();

	info.renderGraph->addPass(*this);
}

void ImagePass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	screenRenderTarget.resize(size.x, size.y);
}
