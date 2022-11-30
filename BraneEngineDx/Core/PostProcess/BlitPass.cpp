#include "BlitPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../GUI/UIControl.h"

BlitPass::BlitPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
}

void BlitPass::prepare()
{
	MaterialRenderData* materialRenderData = (MaterialRenderData*)this->materialRenderData;
	materialRenderData->program = program;
	materialRenderData->create();
}

void BlitPass::execute(IRenderContext& context)
{
	materialRenderData->upload();

	context.bindShaderProgram(program);

	context.bindMaterialBuffer(((MaterialRenderData*)materialRenderData)->vendorMaterial);

	context.bindFrame(resource->finalRenderTarget->getVendorRenderTarget());
	context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), Fragment_Shader_Stage, screenMapSlot);

	context.setDrawInfo(0, 1, 0);
	context.clearFrameColor({ 0, 0, 0, 0 });
	context.clearFrameDepth(1);
	context.setViewport(0, 0, size.x, size.y);
	context.postProcessCall();
}

bool BlitPass::mapMaterialParameter(RenderInfo & info)
{
	if (resource == NULL || resource->finalRenderTarget == NULL)
		return false;
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/BlitPass.mat");
	if (resource == NULL || resource->screenTexture == NULL || resource->finalRenderTarget == NULL)
		return false;
	materialRenderData = material->getRenderData();
	return materialRenderData;
}

void BlitPass::render(RenderInfo & info)
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

	if (!program->isComputable()) {
		program->init();

		screenMapSlot = program->getAttributeOffset("screenMap").offset;
		if (screenMapSlot >= 0)
			info.renderGraph->addPass(*this);
	}
		
}
