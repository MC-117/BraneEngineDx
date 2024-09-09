#include "ImagePass.h"
#include "../Asset.h"
#include "../RenderCore/RenderCore.h"
#include "../Console.h"

ImagePass::ImagePass(const string& name, Material* material) : PostProcessPass(name, material)
{
	screenMap.setAutoGenMip(false);
	screenRenderTarget.addTexture("image", screenMap);
}

void ImagePass::prepare()
{
	materialVaraint = materialRenderData->getVariant(Shader_Postprocess);
	if (materialVaraint == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", materialRenderData->getShaderName().c_str());
		throw runtime_error("ShaderVariant not found");
		return;
	}

	materialVaraint->init();
	
	Texture2D* sceneMap = dynamic_cast<Texture2D*>(resource->screenTexture);
	if (sceneMap)
		screenMap.setTextureInfo(sceneMap->getTextureInfo());
	screenRenderTarget.init();
}

void ImagePass::execute(IRenderContext& context)
{
	context.bindShaderProgram(materialVaraint->program);
	context.bindFrame(screenRenderTarget.getVendorRenderTarget());
	context.bindMaterialTextures(materialVaraint);
	context.setRenderPostState();
	context.postProcessCall();
	context.clearFrameBindings();
	resource->screenTexture = &screenMap;
	resource->screenRenderTarget = &screenRenderTarget;
}

bool ImagePass::loadDefaultResource()
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/Editor/ImagePPTest.mat");
	if (material == NULL || resource == NULL || resource->screenTexture == NULL)
		return false;
	image = *material->getTexture("image");
	materialRenderData = material->getMaterialRenderData();
	return image && materialRenderData;
}

void ImagePass::render(RenderInfo& info)
{
	if (!enable)
		return;
	if (size.x == 0 || size.y == 0)
		return;
	if (!loadDefaultResource())
		return;
	
	RENDER_THREAD_ENQUEUE_TASK(AddImagePass, ([this, materialRenderData = materialRenderData] (RenderThreadContext& context)
	{
		context.renderGraph->addPass(*this);
		if (materialRenderData)
			renderGraph->getRenderDataCollectorMainThread()->add(*materialRenderData);
	}));
}

void ImagePass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	screenRenderTarget.resize(size.x, size.y);
}
