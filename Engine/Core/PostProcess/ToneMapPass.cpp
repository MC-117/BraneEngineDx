#include "ToneMapPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/RenderCore.h"

ToneMapPass::ToneMapPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	screenMap.setAutoGenMip(false);
	screenRenderTarget.addTexture("screenMap", screenMap);
}

void ToneMapPass::prepare()
{
	MaterialRenderData* materialRenderData = (MaterialRenderData*)this->materialRenderData;
	materialRenderData->program = program;
	materialRenderData->create();
}

void ToneMapPass::execute(IRenderContext& context)
{
	materialRenderData->upload();

	context.bindShaderProgram(program);

	context.bindMaterialBuffer(((MaterialRenderData*)materialRenderData)->vendorMaterial);
	context.bindMaterialTextures(((MaterialRenderData*)materialRenderData)->vendorMaterial);

	if (program->isComputable()) {
		static const ShaderPropertyName imageMapName = "imageMap";

		Image image;
		image.texture = resource->screenTexture;

		context.bindImage(image, imageMapName);

		Vector3u localSize = material->getLocalSize();
		localSize.x() = ceilf(size.x / (float)localSize.x());
		localSize.y() = ceilf(size.y / (float)localSize.y());

		context.setDrawInfo(0, 1, 0);
		context.dispatchCompute(localSize.x(), localSize.y(), 1);
	}
	else {
		static const ShaderPropertyName screenMapName = "screenMap";

		context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), screenMapName);
		
		context.bindFrame(screenRenderTarget.getVendorRenderTarget());

		context.setDrawInfo(0, 1, 0);
		context.setRenderPostState();
		context.postProcessCall();

		context.clearFrameBindings();

		resource->screenTexture = &screenMap;
		resource->screenRenderTarget = &screenRenderTarget;
	}
}

bool ToneMapPass::mapMaterialParameter(RenderInfo & info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/ToneMapPassFS.mat");
	if (material == NULL || resource == NULL || resource->screenTexture == NULL)
		return false;
	materialRenderData = material->getRenderData();
	return materialRenderData;
}

void ToneMapPass::render(RenderInfo & info)
{
	if (!enable)
		return;
	if (!mapMaterialParameter(info))
		return;
	if (size.x == 0 || size.y == 0)
		return;
	program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", material->getShaderName());
		return;
	}
	program->init();
	info.renderGraph->addPass(*this);
}

void ToneMapPass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	screenRenderTarget.resize(size.x, size.y);
}
